#include "pichi.h"


#include <iostream>
#include <thread>
#include <algorithm>
#include <utility>
#include <mutex>

#include "logfile.h"
#include "util/util.h"
#include "base/udp_transmitter.h"


Pichi::Pichi(Configuration&& conf)
  : conf_{std::move(conf)},
    nmea_reader_{conf_, timer_, nmea_data_ready_, nmea_data_mutex_, nmea_data_},
    gnss_receiver_{timer_, gnss_data_ready_, gnss_data_mutex_, gnss_data_}
{
  devices_.reserve(32);
  devices_.emplace_back(0);  // 0 should always be the local device

  if (!timer_.systime_init())
    std::cerr << "Values relying on the 1MHz system timer will be zero" << std::endl;
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

    std::thread provider{&nmea::Reader::start, &nmea_reader_};
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

    std::thread consumer{&Pichi::log_gnss_packets, this};
    consumer.detach();

    std::thread provider{&gnss::Receiver::start, &gnss_receiver_,
                         std::ref(conf_.recv_ip), conf_.recv_port};
    provider.detach();
  }
  else
    std::cerr << "Already running!" << std::endl;
}


void Pichi::start_gnss_logger()
{
  if (!is_active()) {
    reset();
    active_.store(true);

    std::thread consumer{&Pichi::log_gnss_data, this};
    consumer.detach();

    std::thread provider{&nmea::Reader::start, &nmea_reader_};
    provider.detach();
  }
  else
    std::cerr << "Already running!" << std::endl;
}


void Pichi::start_gnss_display()
{
  if (!is_active()) {
    reset();
    active_.store(true);

    std::thread consumer{&Pichi::update_gnss_data, this};
    consumer.detach();

    std::thread provider{&nmea::Reader::start, &nmea_reader_};
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

    std::thread provider{&nmea::Reader::start, &nmea_reader_};
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
        decltype(nmea_data_) nmea_reads;
        std::swap(nmea_data_, nmea_reads);
        lk.unlock();

        for (const auto& read : nmea_reads) {
          if (parse_location(location, read)) {
            header->transmit_counter++;
            header->transmit_time = timer_.current_time();
            header->transmit_system_delay = timer_.current_systime() - read.systime;
            transmitter.send(gsl::as_span(buffer).first(packet_size));
            activity_counter_.fetch_add(1);
            set_gnss_location(local_device_id, location);
          }
        }
      }
    }
  }
}


void Pichi::log_gnss_packets()
{
  logging::Logfile file(std::to_string(timer_.current_time()) + ".csv");

  if (file.is_open()) {
    while (is_active()) {
      std::unique_lock<std::mutex> lk{gnss_data_mutex_};
      gnss_data_ready_.wait_for(lk, std::chrono::milliseconds(100),
          [this] { return !gnss_data_.empty() || !active_.load(); });
      if (!gnss_data_.empty()) {
        decltype(gnss_data_) gnss_recvs;
        std::swap(gnss_data_, gnss_recvs);
        lk.unlock();

        for (const auto& recv : gnss_recvs) {
          switch (static_cast<gnss::PacketType>(recv.header.packet_type)) {
            case gnss::PacketType::Location: {
              if (recv.header.data_size == gnss::location_data_size &&
                  recv.header.data_size >= recv.data.size()) {
                auto* location = reinterpret_cast<const gnss::LocationPacket*>(recv.data.data());
                file.write(&recv.header, location, recv.time);
                set_gnss_location(recv.header.device_id, location);
              }
            }
            break;
            default:
            break;
          }
        }

        activity_counter_.fetch_add(gnss_recvs.size());
      }
    }
  }
}


void Pichi::log_gnss_data()
{
  gnss::LocationPacket location;
  logging::Logfile file(std::to_string(timer_.current_time()) + ".csv");

  if (file.is_open()) {
    while (is_active()) {
      std::unique_lock<std::mutex> lk{nmea_data_mutex_};
      nmea_data_ready_.wait_for(lk, std::chrono::milliseconds(100),
          [this] { return !nmea_data_.empty() || !active_.load(); });
      if (!nmea_data_.empty()) {
        decltype(nmea_data_) nmea_reads;
        std::swap(nmea_data_, nmea_reads);
        lk.unlock();

        for (const auto& read : nmea_reads) {
          if (parse_location(&location, read)) {
            file.write(&location, read.time);
            set_gnss_location(local_device_id, &location);
            activity_counter_.fetch_add(1);
          }
        }
      }
    }
  }
}


void Pichi::update_gnss_data()
{
  gnss::LocationPacket location;

  while (is_active()) {
    std::unique_lock<std::mutex> lk{nmea_data_mutex_};
    nmea_data_ready_.wait_for(lk, std::chrono::milliseconds(100),
        [this] { return !nmea_data_.empty() || !active_.load(); });
    if (!nmea_data_.empty()) {
      decltype(nmea_data_) nmea_reads;
      std::swap(nmea_data_, nmea_reads);
      lk.unlock();

      for (const auto& read : nmea_reads) {
        if (parse_location(&location, read)) {
          set_gnss_location(local_device_id, &location);
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
      decltype(nmea_data_) nmea_read;
      std::swap(nmea_data_, nmea_read);
      lk.unlock();
      for (const auto& read : nmea_read)
        std::cout << read.sentence << std::endl;
      activity_counter_.fetch_add(nmea_read.size());
    }
  }
}


bool Pichi::parse_location(gsl::not_null<gnss::LocationPacket*> location,
                           const nmea::ReadData& nmea_read)
{
  bool success = false;

  switch (nmea_read.sentence_type) {
    case nmea::SentenceType::Rmc: {
      nmea::RmcData rmc_data;
      std::tie(success, rmc_data) = nmea::parse_valid<nmea::RmcData>(nmea_read.sentence);
      if (success) {
        location->utc_timestamp = util::as_utc_unix(
            rmc_data.date_year + 2000, rmc_data.date_month, rmc_data.date_day,
            rmc_data.utc_time_hour, rmc_data.utc_time_minute, rmc_data.utc_time_second);
        location->latitude = util::dm_to_decimal(
            rmc_data.degrees_lat, rmc_data.minutes_lat, rmc_data.direction_lat);
        location->longitude = util::dm_to_decimal(
            rmc_data.degrees_long, rmc_data.minutes_long, rmc_data.direction_long);
      }
    }
    break;
    case nmea::SentenceType::Gga: {
      nmea::GgaData gga_data;
      std::tie(success, gga_data) = nmea::parse_valid<nmea::GgaData>(nmea_read.sentence);
      if (success) {
        location->utc_timestamp = 0;  // TODO: GGA has no date, buffer date from RMC?
        location->latitude = util::dm_to_decimal(
            gga_data.degrees_lat, gga_data.minutes_lat, gga_data.direction_lat);
        location->longitude = util::dm_to_decimal(
            gga_data.degrees_long, gga_data.minutes_long, gga_data.direction_long);
      }
    }
    default:
    break;
  }

  return success;
}


std::tuple<bool, gnss::LocationPacket> Pichi::gnss_location(uint16_t device_id)
{
  std::lock_guard<std::mutex> lock{devices_mutex_};

  for (auto& device : devices_) {
    if (device.id() == device_id)
      return std::make_tuple(true, device.location());
  }

  return std::make_tuple(false, gnss::LocationPacket());
}


void Pichi::set_gnss_location(uint16_t device_id,
    gsl::not_null<const gnss::LocationPacket*> location)
{
  std::lock_guard<std::mutex> lock{devices_mutex_};

  auto it = std::find_if(std::begin(devices_), std::end(devices_),
      [device_id](const pichi::Device& d) { return d.id() == device_id; });
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
  std::vector<uint16_t> v;
  std::swap(new_device_ids_, v);
  return v;
}