#include "pichi.h"


#include <memory>
#include <iostream>
#include <thread>
#include <algorithm>
#include <utility>
#include <mutex>
#include <chrono>

#include "../util/util.h"
#include "../base/udp_transmitter.h"
#include "../log/csv_file.h"
#include "../log/gpx_file.h"


namespace
{


void fill_packet(gsl::not_null<pichi::LocationPacket*> location, const nmea::RmcData& rmc_data)
{
  location->utc_timestamp = util::as_utc_unix(rmc_data.date_year + 2000, rmc_data.date_month,
      rmc_data.date_day, rmc_data.utc_time_hour, rmc_data.utc_time_minute,
      rmc_data.utc_time_second);
  location->latitude = util::dm_to_decimal(rmc_data.degrees_lat, rmc_data.minutes_lat,
      rmc_data.direction_lat);
  location->longitude = util::dm_to_decimal(rmc_data.degrees_long, rmc_data.minutes_long,
      rmc_data.direction_long);
}


bool parse_sentence(gsl::not_null<pichi::LocationPacket*> location,
    nmea::RmcData& rmc_data, const pichi::NmeaSentence& nmea_sentence)
{
  if (nmea_sentence.type != nmea::SentenceType::Rmc)
    return false;

  bool success = false;
  std::tie(success, rmc_data) = nmea::parse_valid<nmea::RmcData>(nmea_sentence.text);
  if (success)
    fill_packet(location, rmc_data);
  return success;
}


}  // namespace


pichi::Pichi::Pichi(Configuration&& conf) : conf_{std::move(conf)},
    nmea_reader_{timer_, nmea_data_ready_, nmea_data_mutex_, nmea_data_},
    receiver_{timer_, rx_data_ready_, rx_data_mutex_, rx_data_}
{
  devices_.reserve(32);
  devices_.emplace_back(Device::LOCAL_DEVICE_ID);  // This device itself

  if (!timer_.init_sys_time())
    std::cerr << "Values relying on the 1MHz system timer will be zero" << std::endl;
}


pichi::Pichi::~Pichi()
{
  if (is_active())
    stop();
  conf_.save_to_file();
}


bool pichi::Pichi::set_config(const Configuration& conf)
{
  if (is_active()) {
    std::cerr << "Can't set configuration while running!" << std::endl;
    return false;
  }

  conf_ = conf;
  std::cout << "Configuration updated" << std::endl;
  return true;
}


void pichi::Pichi::reset()
{
  activity_counter_.store(0);

  // Clear data
  {
    std::lock_guard<std::mutex> lock(nmea_data_mutex_);
    decltype(nmea_data_){}.swap(nmea_data_);
  }
  {
    std::lock_guard<std::mutex> lock(rx_data_mutex_);
    decltype(rx_data_){}.swap(rx_data_);
  }
}


void pichi::Pichi::stop()
{
  // Stop async i/o operations
  nmea_reader_.stop();
  receiver_.stop();

  // Disable detached threads
  active_.store(false);

  // Wake up async i/o threads to let them to finish
  nmea_data_ready_.notify_all();
  rx_data_ready_.notify_all();

  // Give detached threads and asio some time to properly finish
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}


void pichi::Pichi::start_transmitter()
{
  if (is_active())
    throw Error{"Already running!"};

  reset();
  active_.store(true);

  std::thread consumer{&pichi::Pichi::transmit_packets, this};
  consumer.detach();

  std::thread provider{&NmeaReader::start, &nmea_reader_, std::ref(conf_.gnss_port),
      conf_.gnss_port_rate};
  provider.detach();
}


void pichi::Pichi::start_receiver()
{
  if (is_active())
    throw Error{"Already running!"};

  reset();
  active_.store(true);

  std::thread consumer{&Pichi::receive_packets, this};
  consumer.detach();

  std::thread provider{&Receiver::start, &receiver_, std::ref(conf_.recv_ip), conf_.recv_port};
  provider.detach();
}


void pichi::Pichi::start_logger()
{
  if (is_active())
    throw Error{"Already running!"};

  reset();
  active_.store(true);

  std::thread consumer{&Pichi::log_location, this};
  consumer.detach();

  std::thread provider{&NmeaReader::start, &nmea_reader_, std::ref(conf_.gnss_port),
      conf_.gnss_port_rate};
  provider.detach();
}


void pichi::Pichi::start_device()
{
  if (is_active())
    throw Error{"Already running!"};

  reset();
  active_.store(true);

  std::thread consumer{&Pichi::update_location, this};
  consumer.detach();

  std::thread provider{&NmeaReader::start, &nmea_reader_, std::ref(conf_.gnss_port),
      conf_.gnss_port_rate};
  provider.detach();
}


void pichi::Pichi::start_debug_mode()
{
  if (is_active())
    throw Error{"Already running!"};

  reset();
  active_.store(true);

  std::thread consumer{&Pichi::print_nmea_sentences, this};
  consumer.detach();

  std::thread provider{&NmeaReader::start, &nmea_reader_, std::ref(conf_.gnss_port),
      conf_.gnss_port_rate};
  provider.detach();
}


void pichi::Pichi::transmit_packets()
{
  base::UdpTransmitter transmitter{};
  if (!transmitter.open(conf_.trans_ip, conf_.trans_port))
    return;

  constexpr auto PACKET_SIZE = PACKET_HEADER_SIZE + LOCATION_PACKET_SIZE;
  std::array<std::uint8_t, PACKET_SIZE> buffer;

  auto* header = reinterpret_cast<PacketHeader*>(buffer.data());
  auto* location = reinterpret_cast<LocationPacket*>(buffer.data() + PACKET_HEADER_SIZE);

  header->packet_type = static_cast<std::uint16_t>(PacketType::Location);
  header->data_size = LOCATION_PACKET_SIZE;
  header->device_id = conf_.device_id;
  header->transmit_counter = 0;

  nmea::RmcData rmc_data;  // Buffer for parsing

  while (is_active()) {
    std::unique_lock<std::mutex> lock{nmea_data_mutex_};
    nmea_data_ready_.wait_for(lock, std::chrono::milliseconds(100),
        [this] { return !nmea_data_.empty() || !active_.load(); });
    if (nmea_data_.empty())
      continue;
    auto nmea_sentences = util::take(nmea_data_);
    lock.unlock();
    for (const auto& sentence : nmea_sentences) {
      if (parse_sentence(location, rmc_data, sentence)) {
        header->transmit_counter++;
        header->transmit_time = timer_.current_unix_time();
        header->transmit_system_delay = timer_.current_sys_time() - sentence.read_time.sys_time;
        transmitter.send(gsl::as_span(buffer).first(PACKET_SIZE));
        
        activity_counter_.fetch_add(1);
        set_location(Device::LOCAL_DEVICE_ID, location);
      }
    }
  }
}


void pichi::Pichi::receive_packets()
{
  std::unique_ptr<CsvFile> csv{nullptr};
  if (conf_.recv_log) {
    //csv = std::make_unique<CsvFile>{};
    //if (!csv->open(std::to_string(timer_.current_unix_time()) + ".csv"))
    //  csv = nullptr;
  }

  while (is_active()) {
    std::unique_lock<std::mutex> lock{rx_data_mutex_};
    rx_data_ready_.wait_for(lock, std::chrono::milliseconds(100),
        [this] { return !rx_data_.empty() || !active_.load(); });
    if (rx_data_.empty())
      continue;
    auto rx_data = util::take(rx_data_);
    lock.unlock();
    for (const auto& rx : rx_data) {
      handle_receive(rx, csv.get());
    }

    activity_counter_.fetch_add(rx_data.size());
  }
}


void pichi::Pichi::log_location()
{
  LocationPacket location;
  CsvFile csv;
  GpxFile gpx;

  std::string basename = std::string("logs/log_" + std::to_string(timer_.current_unix_time()));
  bool logging_csv = conf_.log_csv && csv.open(basename + ".csv");
  bool logging_gpx = conf_.log_gpx && gpx.open(basename + ".gpx");
  
  auto seconds_now = []() -> std::uint64_t {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();
    return static_cast<std::uint64_t>(seconds);
  };

  auto last_write = seconds_now();

  while (is_active()) {
    std::unique_lock<std::mutex> lock{nmea_data_mutex_};
    nmea_data_ready_.wait_for(lock, std::chrono::milliseconds(100),
        [this] { return !nmea_data_.empty() || !active_.load(); });
    if (nmea_data_.empty())
      continue;
    auto nmea_sentences = util::take(nmea_data_);
    lock.unlock();
    auto cur_time = seconds_now();
    for (const auto& sentence : nmea_sentences) {
      nmea::RmcData rmc_data;
      if (parse_sentence(&location, rmc_data, sentence)) {
        if (logging_csv && (!conf_.log_csv_force_1hz || cur_time != last_write)) {
          csv.write(&location, sentence.read_time.unix_time);
        }
        if (logging_gpx && (!conf_.log_gpx_force_1hz || cur_time != last_write)) {
          gpx.write_trackpoint(location.latitude, location.longitude,
              rmc_data.date_year + 2000, rmc_data.date_month, rmc_data.date_day,
              rmc_data.utc_time_hour, rmc_data.utc_time_minute, rmc_data.utc_time_second);
        }
        last_write = cur_time;

        set_location(Device::LOCAL_DEVICE_ID, &location);
        activity_counter_.fetch_add(1);
      }
    }
  }
}


void pichi::Pichi::update_location()
{
  LocationPacket location;
  nmea::RmcData rmc_data;

  while (is_active()) {
    std::unique_lock<std::mutex> lk{nmea_data_mutex_};
    nmea_data_ready_.wait_for(lk, std::chrono::milliseconds(100),
        [this] { return !nmea_data_.empty() || !active_.load(); });
    if (!nmea_data_.empty()) {
      auto data = util::take(nmea_data_);
      lk.unlock();
      for (const auto& sentence : data) {
        if (parse_sentence(&location, rmc_data, sentence)) {
          set_location(Device::LOCAL_DEVICE_ID, &location);
          activity_counter_.fetch_add(1);
        }
      }
    }
  }
}


void pichi::Pichi::print_nmea_sentences()
{
  while (is_active()) {
    std::unique_lock<std::mutex> lk{nmea_data_mutex_};
    nmea_data_ready_.wait_for(lk, std::chrono::milliseconds(100),
        [this] { return !nmea_data_.empty() || !active_.load(); });
    if (!nmea_data_.empty()) {
      auto data = util::take(nmea_data_);
      lk.unlock();
      for (const auto& sentence : data)
        std::cout << sentence.text;
      activity_counter_.fetch_add(data.size());
    }
  }
}


void pichi::Pichi::handle_receive(const ReceiveData& rx, CsvFile* csv)
{
  bool valid = static_cast<PacketType>(rx.header.packet_type) == PacketType::Location &&
      rx.header.data_size == LOCATION_PACKET_SIZE && rx.header.data_size >= rx.data.size();

  if (valid) {
    auto* location = reinterpret_cast<const LocationPacket*>(rx.data.data());
    set_location(rx.header.device_id, location);

    if (csv) {
      switch (conf_.recv_log_format) {
        case LogFormat::Full:
          csv->write(&rx.header, location, rx.receive_time.unix_time);
        break;
        case LogFormat::Short:
          csv->write(location, rx.header.device_id, rx.receive_time.unix_time);
        break;
      }
    }
  }
}


auto pichi::Pichi::get_location(std::uint16_t device_id) -> std::tuple<bool, LocationPacket>
{
  std::lock_guard<std::mutex> lock{devices_mutex_};

  for (auto& device : devices_) {
    if (device.id() == device_id)
      return std::make_tuple(true, device.location());
  }

  return std::make_tuple(false, LocationPacket{});
}


void pichi::Pichi::set_location(std::uint16_t device_id,
    gsl::not_null<const LocationPacket*> location)
{
  std::lock_guard<std::mutex> lock{devices_mutex_};

  auto it = std::find_if(std::begin(devices_), std::end(devices_),
      [device_id](const auto& device) { return device.id() == device_id; });

  if (it != std::end(devices_)) {
    it->set_location(*location);
  }
  else {
    devices_.emplace_back(device_id);
    devices_.back().set_location(*location);
  }
}


std::vector<std::uint16_t> pichi::Pichi::get_device_ids()
{
  std::lock_guard<std::mutex> lock{devices_mutex_};

  std::vector<std::uint16_t> ids;
  for (const auto& dev : devices_)
    ids.push_back(dev.id());

  return ids;
}
