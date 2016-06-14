#include "pichi.h"


#include <iostream>
#include <thread>


Pichi::Pichi(Configuration&& conf)
  : conf_{std::move(conf)},
    nmea_reader_{conf_, timer_, nmea_data_ready_, nmea_data_mutex_, nmea_data_},
    gnss_transceiver_{conf_, timer_, gnss_data_ready_, gnss_data_mutex_, gnss_data_}
{
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
    std::cerr << "Can't set configuration while transceiver is running!" << std::endl;
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

  if (gnss_transceiver_.is_running())
    gnss_transceiver_.stop();

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

    std::thread consumer{&gnss::Transceiver::start_transmitter, &gnss_transceiver_};
    consumer.detach();

    std::thread provider{&nmea::Reader::start, &nmea_reader_};
    provider.detach();
  }
  else std::cerr << "Transceiver is already running!" << std::endl;
}


void Pichi::start_gnss_receiver()
{
  if (!is_active()) {
    reset();
    active_.store(true);

    std::thread consumer{&Pichi::log_gnss_data, this};
    consumer.detach();

    std::thread provider{&gnss::Transceiver::start_receiver, &gnss_transceiver_};
    provider.detach();
  }
  else std::cerr << "Transceiver is already running!" << std::endl;
}


void Pichi::start_gnss_logger()
{
  if (!is_active()) {
    reset();
    active_.store(true);

    std::thread consumer{&Pichi::log_nmea_data, this};
    consumer.detach();

    std::thread provider{&nmea::Reader::start, &nmea_reader_};
    provider.detach();
  }
  else std::cerr << "Transceiver is already running!" << std::endl;
}


void Pichi::log_gnss_data()
{
  while (is_active()) {
    std::unique_lock<std::mutex> lk{gnss_data_mutex_};
    gnss_data_ready_.wait_for(lk, std::chrono::milliseconds(100),
        [this] { return !gnss_data_.empty() || !active_.load(); });
    if (!gnss_data_.empty()) {
      decltype(gnss_data_) data;
      std::swap(gnss_data_, data);
      lk.unlock();
      activity_counter_.fetch_add(data.size());
    }
  }
}


void Pichi::log_nmea_data()
{
  while (is_active()) {
    std::unique_lock<std::mutex> lk{nmea_data_mutex_};
    nmea_data_ready_.wait_for(lk, std::chrono::milliseconds(100),
        [this] { return !nmea_data_.empty() || !active_.load(); });
    if (!nmea_data_.empty()) {
      decltype(nmea_data_) data;
      std::swap(nmea_data_, data);
      lk.unlock();
      
      while (!data.empty()) {
        auto nmea_data = std::move(data.front());
        data.pop_front();
        switch (nmea_data.sentence_type) {
          case nmea::SentenceType::Rmc: {
            bool success = false;
            nmea::RmcData rmc_data;
            uint8_t crc;
            const auto& s = nmea_data.sentence;
            std::tie(success, rmc_data, crc) = nmea::parse<nmea::RmcData>(s);
            if (success && nmea::comp_checksum(s, crc)) {
              activity_counter_.fetch_add(1);
            }
          }
          break;
          default:
          break;
        }
      }
    }
  }
}
