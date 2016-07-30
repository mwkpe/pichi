#include "pichi.h"


#include <iostream>
#include <thread>
#include <algorithm>
#include <utility>
#include <mutex>
#include <chrono>

#include "util/util.h"
#include "base/udp_transmitter.h"
#include "logging/csvfile.h"
#include "logging/gpxfile.h"


Pichi::Pichi(Configuration&& conf)
  : conf_{std::move(conf)},
    nmea_reader_{timer_, nmea_data_ready_, nmea_data_mutex_, nmea_data_},
    gnss_receiver_{timer_, gnss_data_ready_, gnss_data_mutex_, gnss_data_}
{
  devices_.reserve(32);
  devices_.emplace_back(0);  // 0 should always be the local device

  if (!timer_.systime_init())
    std::cerr << "Values relying on the 1MHz system timer will be zero"
              << std::endl;
}


Pichi::~Pichi()
{
  if (is_active())
    stop();
  conf_.save_to_file();

  // Give detached threads and asio some time to properly finish
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}


bool Pichi::set_config(const Configuration& conf)
{
  if (is_active()) {
    std::cerr << "Can't set configuration while running!" << std::endl;
    return false;
  }

  conf_ = conf;
  std::cout << "Configuration updated" << std::endl;
  return true;
}


void Pichi::reset()
{
  activity_counter_.store(0);
  {
    std::lock_guard<std::mutex> lk(nmea_data_mutex_);
    decltype(nmea_data_){}.swap(nmea_data_);
  }
  {
    std::lock_guard<std::mutex> lk(gnss_data_mutex_);
    decltype(gnss_data_){}.swap(gnss_data_);
  }
}


void Pichi::stop()
{
  if (nmea_reader_.is_running())
    nmea_reader_.stop();

  if (gnss_receiver_.is_running())
    gnss_receiver_.stop();

  // Disable detached threads
  active_.store(false);

  // Wake up threads to let them to finish
  nmea_data_ready_.notify_all();
  gnss_data_ready_.notify_all();
}


void Pichi::start_gnss_transmitter()
{
  if (!is_active()) {
    reset();
    active_.store(true);

    std::thread consumer{&Pichi::transmit_gnss_packets, this};
    consumer.detach();

    std::thread provider{
        &nmea::Reader::start, &nmea_reader_,
        std::ref(conf_.gnss_port), conf_.gnss_port_rate
    };
    provider.detach();
  }
  else
    std::cerr << "Already running!" << std::endl;
}


void Pichi::start_gnss_receiver()
{
  if (!is_active()) {
    reset();
    active_.store(true);

    std::thread consumer{&Pichi::receive_gnss_packets, this};
    consumer.detach();

    std::thread provider{
        &gnss::Receiver::start, &gnss_receiver_,
        std::ref(conf_.recv_ip), conf_.recv_port
    };
    provider.detach();
  }
  else
    std::cerr << "Already running!" << std::endl;
}


void Pichi::start_location_logger()
{
  if (!is_active()) {
    reset();
    active_.store(true);

    std::thread consumer{&Pichi::log_position, this};
    consumer.detach();

    std::thread provider{
        &nmea::Reader::start, &nmea_reader_,
        std::ref(conf_.gnss_port), conf_.gnss_port_rate
    };
    provider.detach();
  }
  else
    std::cerr << "Already running!" << std::endl;
}


void Pichi::start_location_display()
{
  if (!is_active()) {
    reset();
    active_.store(true);

    std::thread consumer{&Pichi::update_position, this};
    consumer.detach();

    std::thread provider{
        &nmea::Reader::start, &nmea_reader_,
        std::ref(conf_.gnss_port), conf_.gnss_port_rate
    };
    provider.detach();
  }
  else
    std::cerr << "Already running!" << std::endl;
}


void Pichi::start_debugger()
{
  if (!is_active()) {
    reset();
    active_.store(true);

    std::thread consumer{&Pichi::show_nmea_sentences, this};
    consumer.detach();

    std::thread provider{
      &nmea::Reader::start, &nmea_reader_,
      std::ref(conf_.gnss_port), conf_.gnss_port_rate
    };
    provider.detach();
  }
  else
    std::cerr << "Already running!" << std::endl;
}


void Pichi::transmit_gnss_packets()
{
  udp::Transmitter transmitter{};

  if (transmitter.open(conf_.trans_ip, conf_.trans_port)) {
    const auto packet_size = gnss::packet_header_size + gnss::location_data_size;
    std::array<uint8_t, packet_size> buffer;

    auto* header = reinterpret_cast<gnss::PacketHeader*>(buffer.data());
    auto* location = reinterpret_cast<gnss::LocationPacket*>(
        buffer.data() + gnss::location_data_size);

    header->packet_type = static_cast<uint16_t>(gnss::PacketType::Location);
    header->data_size = gnss::location_data_size;
    header->device_id = conf_.device_id;
    header->transmit_counter = 0;

    while (is_active()) {
      std::unique_lock<std::mutex> lk{nmea_data_mutex_};
      nmea_data_ready_.wait_for(lk, std::chrono::milliseconds(100),
          [this] { return !nmea_data_.empty() || !active_.load(); });
      if (!nmea_data_.empty()) {
        auto data = util::take(nmea_data_);
        lk.unlock();
        for (const auto& sentence : data) {
          if (parse_location(location, sentence)) {
            header->transmit_counter++;
            header->transmit_time = timer_.current_time();
            header->transmit_system_delay = timer_.current_systime() - sentence.systime;
            transmitter.send(gsl::as_span(buffer).first(packet_size));
            activity_counter_.fetch_add(1);
            set_device_location(local_device_id, location);
          }
        }
      }
    }
  }
}


void Pichi::receive_gnss_packets()
{
  logging::CsvFile csv;
  bool logging = conf_.recv_log &&
                 csv.open(std::to_string(timer_.current_time()) + ".csv");

  while (is_active()) {
    std::unique_lock<std::mutex> lk{gnss_data_mutex_};
    gnss_data_ready_.wait_for(lk, std::chrono::milliseconds(100),
        [this] { return !gnss_data_.empty() || !active_.load(); });
    if (gnss_data_.empty()) {
      continue;
    }
    auto data = util::take(gnss_data_);
    lk.unlock();
    for (const auto& recv : data) {
      switch (static_cast<gnss::PacketType>(recv.header.packet_type)) {
        case gnss::PacketType::Location: {
          if (recv.header.data_size == gnss::location_data_size &&
              recv.header.data_size >= recv.data.size()) {
            using lp = gnss::LocationPacket;
            auto* location = reinterpret_cast<const lp*>(recv.data.data());
            set_device_location(recv.header.device_id, location);
            if (logging) {
              switch (conf_.recv_log_format) {
                case LogFormat::Full:
                  csv.write(&recv.header, location, recv.time);
                break;
                case LogFormat::Short:
                  csv.write(location, recv.header.device_id, recv.time);
                break;
              }
            }
          }
        }
        break;
        default:
        break;
      }
    }

    activity_counter_.fetch_add(data.size());
  }
}


void Pichi::log_position()
{
  gnss::LocationPacket location;
  logging::CsvFile csv;
  logging::GpxFile gpx;
  auto t = timer_.current_time();
  std::string basename = std::string("logs/log_" + std::to_string(t));
  bool logging_csv = conf_.log_csv && csv.open(basename + ".csv");
  bool logging_gpx = conf_.log_gpx && gpx.open(basename + ".gpx");
  
  auto seconds_now = []() -> uint64_t {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();
    return static_cast<uint64_t>(seconds);
  };
  auto last_write = seconds_now();

  while (is_active()) {
    std::unique_lock<std::mutex> lk{nmea_data_mutex_};
    nmea_data_ready_.wait_for(lk, std::chrono::milliseconds(100),
        [this] { return !nmea_data_.empty() || !active_.load(); });
    if (nmea_data_.empty())
      continue;
    auto data = util::take(nmea_data_);
    lk.unlock();
    for (const auto& sentence : data) {
      nmea::RmcData rmc_data;
      if (sentence.type == nmea::SentenceType::Rmc &&
          parse_location(&location, sentence.text, rmc_data)) {

        auto cur_time = seconds_now();
        bool allow_write = cur_time > last_write;
        if (allow_write)
          last_write = cur_time;

        if (logging_csv && (!conf_.log_csv_force_1hz || allow_write)) {
          csv.write(&location, sentence.time);
        }

        if (logging_gpx && (!conf_.log_gpx_force_1hz || allow_write)) {
          gpx.write_trackpoint(
              location.latitude,
              location.longitude,
              rmc_data.date_year + 2000,
              rmc_data.date_month,
              rmc_data.date_day,
              rmc_data.utc_time_hour,
              rmc_data.utc_time_minute,
              rmc_data.utc_time_second);
        }

        set_device_location(local_device_id, &location);
        activity_counter_.fetch_add(1);
      }
    }
  }
}


void Pichi::update_position()
{
  gnss::LocationPacket location;

  while (is_active()) {
    std::unique_lock<std::mutex> lk{nmea_data_mutex_};
    nmea_data_ready_.wait_for(lk, std::chrono::milliseconds(100),
        [this] { return !nmea_data_.empty() || !active_.load(); });
    if (!nmea_data_.empty()) {
      auto data = util::take(nmea_data_);
      lk.unlock();
      for (const auto& sentence : data) {
        if (parse_location(&location, sentence)) {
          set_device_location(local_device_id, &location);
          activity_counter_.fetch_add(1);
        }
      }
    }
  }
}


void Pichi::show_nmea_sentences()
{
  while (is_active()) {
    std::unique_lock<std::mutex> lk{nmea_data_mutex_};
    nmea_data_ready_.wait_for(lk, std::chrono::milliseconds(100),
        [this] { return !nmea_data_.empty() || !active_.load(); });
    if (!nmea_data_.empty()) {
      auto data = util::take(nmea_data_);
      lk.unlock();
      for (const auto& sentence : data)
        std::cout << sentence.text << std::endl;
      activity_counter_.fetch_add(data.size());
    }
  }
}


bool Pichi::parse_location(
    gsl::not_null<gnss::LocationPacket*> location,
    const nmea::Sentence& sentence)
{
  bool success = false;

  switch (sentence.type) {
    case nmea::SentenceType::Rmc: {
      nmea::RmcData rmc_data;
      std::tie(success, rmc_data) = nmea::parse_valid<nmea::RmcData>(sentence.text);
      if (success)
        set_location(location, rmc_data);
    }
    break;
    default:
    break;
  }

  return success;
}


bool Pichi::parse_location(
    gsl::not_null<gnss::LocationPacket*> location,
    const std::string& nmea_sentence,
    nmea::RmcData& rmc_data)
{
  bool success = false;
  std::tie(success, rmc_data) = nmea::parse_valid<nmea::RmcData>(nmea_sentence);
  if (success)
    set_location(location, rmc_data);
  return success;
}


std::tuple<bool, gnss::LocationPacket> Pichi::gnss_location(uint16_t device_id)
{
  std::lock_guard<std::mutex> lock{devices_mutex_};

  for (auto& device : devices_) {
    if (device.id() == device_id)
      return std::make_tuple(true, device.location());
  }

  return std::make_tuple(false, gnss::LocationPacket{});
}


void Pichi::set_location(
    gsl::not_null<gnss::LocationPacket*> location,
    const nmea::RmcData& rmc_data)
{
  location->utc_timestamp = util::as_utc_unix(
      rmc_data.date_year + 2000, rmc_data.date_month, rmc_data.date_day,
      rmc_data.utc_time_hour, rmc_data.utc_time_minute, rmc_data.utc_time_second);
  location->latitude = util::dm_to_decimal(
      rmc_data.degrees_lat, rmc_data.minutes_lat, rmc_data.direction_lat);
  location->longitude = util::dm_to_decimal(
      rmc_data.degrees_long, rmc_data.minutes_long, rmc_data.direction_long);
}


void Pichi::set_device_location(
    uint16_t device_id,
    gsl::not_null<const gnss::LocationPacket*> location)
{
  std::lock_guard<std::mutex> lock{devices_mutex_};

  auto it = std::find_if(std::begin(devices_), std::end(devices_),
      [device_id](const auto& device) { return device.id() == device_id; });
  if (it != std::end(devices_)) {
    it->set_location(*location);
  }
  else {
    new_device_ids_.push_back(device_id);
    devices_.emplace_back(device_id);
    devices_.back().set_location(*location);
  }
}


std::vector<uint16_t> Pichi::new_device_ids()
{
  std::lock_guard<std::mutex> lock{devices_mutex_};
  return util::take(new_device_ids_);
}
