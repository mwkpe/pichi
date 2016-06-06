#include "gnss_transceiver.h"


#include <iostream>
#include <ios>
#include <sstream>
#include <fstream>
#include <vector>
#include <deque>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cstdlib>

#include <asio.hpp>

#include "nmea_parser.h"


gnss::Transceiver::Transceiver(Configuration&& conf) : conf_(std::move(conf))
{
  if (!timer.st_init())
    std::cerr << "Data relating to the microsecond system timer will be 0." << std::endl;
}


gnss::Transceiver::~Transceiver()
{
  if (is_active())
    stop();
  conf_.save_to_file();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}


void gnss::Transceiver::reset()
{
  std::lock_guard<std::mutex> lk(data_mutex_);
  decltype(nmea_sentences_){}.swap(nmea_sentences_);
  activity_counter_.store(0);
}


void gnss::Transceiver::start_gnss_transmitter()
{
  reset();
  active_.store(true);

  std::thread consumer{&gnss::Transceiver::transmit_gnss_packets, this};
  consumer.detach();

  std::thread provider{&gnss::Transceiver::read_nmea_sentences, this};
  provider.detach();
}


void gnss::Transceiver::start_gnss_receiver(const std::string& logfilename)
{
  reset();
  active_.store(true);
  
  std::thread t{&gnss::Transceiver::receive_gnss_packets, this, logfilename};
  t.detach();
}


void gnss::Transceiver::start_nmea_logger(const std::string& logfilename)
{
  reset();
  active_.store(true);

  std::thread consumer{&gnss::Transceiver::log_nmea_sentences, this, logfilename};
  consumer.detach();

  std::thread provider{&gnss::Transceiver::read_nmea_sentences, this};
  provider.detach();
}


void gnss::Transceiver::start_gnss_logger(const std::string& logfilename)
{
  reset();
  active_.store(true);

  std::thread consumer{&gnss::Transceiver::log_gnss_data, this, logfilename};
  consumer.detach();

  std::thread provider{&gnss::Transceiver::read_nmea_sentences, this};
  provider.detach();
}


void gnss::Transceiver::stop()
{
  active_.store(false);
  data_cond_.notify_all();
}


void gnss::Transceiver::read_nmea_sentences()
{
  using asio::ip::udp;
  asio::io_service io_service;
  asio::serial_port serial{io_service};
  asio::error_code ec;
  serial.open(conf_.gnss_port, ec);
  if (ec) {
    std::cerr << "Could not open port: " << conf_.gnss_port 
              << '\n' << ec.message() << std::endl;
    active_.store(false);
  }
  else {
    using spbase = asio::serial_port_base;
    serial.set_option(spbase::baud_rate(115200));
    serial.set_option(spbase::character_size(8));
    serial.set_option(spbase::stop_bits(asio::serial_port_base::stop_bits::one));
    serial.set_option(spbase::parity(asio::serial_port_base::parity::none));
    serial.set_option(spbase::flow_control(asio::serial_port_base::flow_control::none));
  }

  while (active_.load()) {
    // Read characters into buffer
    std::array<char, 1536> recv_buffer{};

    // Blocks until read or error
    auto recv_bytes = serial.read_some(asio::buffer(recv_buffer));
    auto recv_st = current_systime();
    auto recv_time = current_time();

    if (recv_bytes > 0) {
      // Prevent crash due to spirit isascii assert when garbage was received
      replace_nonascii(gsl::as_span(recv_buffer), '0');

      // Look for sentences in the buffer
      std::vector<std::string> sentences;
      auto pos = std::begin(recv_buffer);
      auto last = std::begin(recv_buffer);
      while (pos != std::end(recv_buffer)) {
        pos = std::find_if(pos, std::end(recv_buffer), [](char c) { return c == '\r'; });
        if (pos != std::end(recv_buffer)) {
          sentences.push_back({last, pos});
          std::advance(pos, 2);  // Skip carriage return and newline
          last = pos;
        }
      }

      // Push data and notifiy other threads
      if (!sentences.empty()) {
        auto filtered_sentences = filter(std::move(sentences), conf_);
        std::lock_guard<std::mutex> lk(data_mutex_);
        for (auto& t : filtered_sentences) {
          nmea_sentences_.emplace(recv_st, recv_time, std::get<0>(t), std::move(std::get<1>(t)));
        }
        data_cond_.notify_one();
      }
    }
  }

  serial.close();
}


void gnss::Transceiver::receive_gnss_packets(const std::string& logfilename)
{
  std::ofstream logfile;
  if (conf_.log_recv) {
    logfile.open(logfilename);
    if (!logfile.is_open()) {
      std::cerr << "Error opening logfile: " << logfilename << std::endl;
      active_.store(false);
      return;
    }
  }

  using asio::ip::udp;
  asio::io_service io_service;
  udp::resolver resolver{io_service};
  udp::resolver::query query{udp::v4(), conf_.recv_ip.c_str(),
                             std::to_string(conf_.recv_port).c_str()};
  udp::endpoint receiver_ep = *resolver.resolve(query);
  udp::socket socket{io_service};
  asio::error_code ec;
  socket.open(udp::v4(), ec);
  if (ec) {
    std::cerr << "Could not open UDP socket: " << ec.message() << std::endl;
    active_.store(false);
  }
  else {
    socket.bind(receiver_ep);  // Tell stack which packets we want
  }

  uint64_t packet_counter = 0;

  while (active_.load()) {
    std::array<char, 512> recv_buffer{};
    // Blocks until receive or error
    auto received_bytes = socket.receive(asio::buffer(recv_buffer));
    if (received_bytes > sizeof(PacketHeader)) {
      uint64_t receive_time = current_time();
      auto* header = reinterpret_cast<PacketHeader*>(recv_buffer.data());
      
      switch (static_cast<PacketType>(header->data_type)) {
        case PacketType::GxRmc: {
          if (received_bytes >= sizeof(PacketHeader) + sizeof(nmea::RmcData) &&
              header->data_length == sizeof(nmea::RmcData)) {
            auto* data = reinterpret_cast<nmea::RmcData*>(recv_buffer.data() +
                                                          sizeof(PacketHeader));
            if (logfile.is_open()) {
              write_gnss_recv(logfile, header, data, receive_time);
            }
            packet_counter++;
          }
        }
        break;

        case PacketType::GxGga: {
          if (received_bytes >= sizeof(PacketHeader) + sizeof(nmea::GgaData) &&
              header->data_length == sizeof(nmea::GgaData)) {
            auto* data = reinterpret_cast<nmea::GgaData*>(recv_buffer.data() +
                                                          sizeof(PacketHeader));
            if (logfile.is_open()) {
              write_gnss_recv(logfile, header, data, receive_time);
            }
            packet_counter++;
          }
        }
        break;

        default:
        break;
      }

      activity_counter_.store(packet_counter);
    }
  }

  activity_counter_.store(0);
  socket.close();
}


void gnss::Transceiver::transmit_gnss_packets()
{
  using asio::ip::udp;
  asio::io_service io_service;
  udp::resolver resolver{io_service};
  udp::socket socket{io_service};
  udp::resolver::query query{udp::v4(), conf_.trans_ip.c_str(),
                             std::to_string(conf_.trans_port).c_str()};
  udp::endpoint receiver_ep = *resolver.resolve(query);
  asio::error_code ec;
  socket.open(udp::v4(), ec);
  if (ec) {
    std::cerr << "Could not open UDP socket: " << ec.message() << std::endl;
    active_.store(false);
    return;
  }

  uint64_t packet_counter = 0;

  char rmc_buffer[sizeof(PacketHeader) + sizeof(nmea::RmcData)] = {0};
  auto* rmc_header = reinterpret_cast<PacketHeader*>(rmc_buffer);
  rmc_header->data_type = static_cast<uint16_t>(PacketType::GxRmc);
  rmc_header->data_length = sizeof(nmea::RmcData);
  rmc_header->device_id = conf_.device_id;
  auto* rmc_data = reinterpret_cast<nmea::RmcData*>(rmc_buffer + sizeof(PacketHeader));

  char gga_buffer[sizeof(PacketHeader) + sizeof(nmea::GgaData)] = {0};
  auto* gga_header = reinterpret_cast<PacketHeader*>(gga_buffer);
  gga_header->data_type = static_cast<uint16_t>(PacketType::GxGga);
  gga_header->data_length = sizeof(nmea::GgaData);
  gga_header->device_id = conf_.device_id;
  auto* gga_data = reinterpret_cast<nmea::GgaData*>(gga_buffer + sizeof(PacketHeader));

  while (active_.load()) {
    std::unique_lock<std::mutex> lk{data_mutex_};
    data_cond_.wait_for(lk, std::chrono::milliseconds(100),
        [this] { return !nmea_sentences_.empty() || !active_.load(); });
    if (!nmea_sentences_.empty()) {
      decltype(nmea_sentences_) sentences;
      std::swap(nmea_sentences_, sentences);
      lk.unlock();
      while (!sentences.empty()) {
        int64_t recv_st;
        nmea::SentenceType type;
        std::string s;
        std::tie(recv_st, std::ignore, type, s) = std::move(sentences.front());
        sentences.pop();
        uint8_t crc;
        switch (type) {
          case nmea::SentenceType::Rmc: {
            if (nmea::parse(s, rmc_data, &crc) && nmea::comp_checksum(s, crc)) {
              rmc_header->transmit_counter++;
              rmc_header->transmit_time = current_time();
              rmc_header->transmit_system_delay = static_cast<uint32_t>(current_systime() - recv_st);
              socket.send_to(asio::buffer(rmc_buffer), receiver_ep);
              packet_counter++;
            }
          }
          break;

          case nmea::SentenceType::Gga: {
            if (nmea::parse(s, gga_data, &crc) && nmea::comp_checksum(s, crc)) {
              gga_header->transmit_counter++;
              gga_header->transmit_time = current_time();
              gga_header->transmit_system_delay = static_cast<uint32_t>(current_systime() - recv_st);
              socket.send_to(asio::buffer(gga_buffer), receiver_ep);
              packet_counter++;
            }
          }
          break;

          default:
          break;
        }
      }

      activity_counter_.store(packet_counter);
    }
  }

  activity_counter_.store(0);
  socket.close();
}


// Logs NMEA sentences received from a GNSS receiver
void gnss::Transceiver::log_nmea_sentences(const std::string& logfilename)
{
  std::ofstream logfile{logfilename};
  if (!logfile.is_open()) {
    std::cerr << "Error opening logfile: " << logfilename << std::endl;
    active_.store(false);
  }

  uint64_t sentence_counter = 0;

  while (active_.load()) {
    std::unique_lock<std::mutex> lk{data_mutex_};
    data_cond_.wait_for(lk, std::chrono::milliseconds(100),
        [this] { return !nmea_sentences_.empty() || !active_.load(); });
    if (!nmea_sentences_.empty()) {
      decltype(nmea_sentences_) sentences;
      std::swap(nmea_sentences_, sentences);
      lk.unlock();
      while (!sentences.empty()) {
        auto t = std::move(sentences.front());
        sentences.pop();
        logfile << std::get<3>(t) << '\n';
        activity_counter_.store(++sentence_counter);
      }
    }
  }

  activity_counter_.store(0);
}


// Logs data received from a GNSS receiver (parsed NMEA sentences)
void gnss::Transceiver::log_gnss_data(const std::string& logfilename)
{
  std::ofstream logfile{logfilename};
  if (!logfile.is_open()) {
    std::cerr << "Error opening logfile: " << logfilename << std::endl;
    active_.store(false);
  }

  uint64_t data_counter = 0;

  while (active_.load()) {
    std::unique_lock<std::mutex> lk{data_mutex_};
    data_cond_.wait_for(lk, std::chrono::milliseconds(100),
        [this] { return !nmea_sentences_.empty() || !active_.load(); });
    if (!nmea_sentences_.empty()) {
      decltype(nmea_sentences_) sentences;
      std::swap(nmea_sentences_, sentences);
      lk.unlock();
      while (!sentences.empty()) {
        uint64_t recv_time;
        uint64_t recv_st;
        nmea::SentenceType type;
        std::string s;
        std::tie(recv_st, recv_time, type, s) = std::move(sentences.front());
        sentences.pop();
        switch (type) {
          case nmea::SentenceType::Rmc: {
            bool success = false;
            nmea::RmcData data;
            uint8_t crc;
            std::tie(success, data, crc) = nmea::parse<nmea::RmcData>(s);
            if (success && nmea::comp_checksum(s, crc)) {
              write_gnss_read(logfile, &data, recv_time, current_systime() - recv_st);
              data_counter++;
            }
          }
          break;
          case nmea::SentenceType::Gga: {
            bool success = false;
            nmea::GgaData data;
            uint8_t crc;
            std::tie(success, data, crc) = nmea::parse<nmea::GgaData>(s);
            if (success && nmea::comp_checksum(s, crc)) {
              write_gnss_read(logfile, &data, recv_time, current_systime() - recv_st);
              data_counter++;
            }
          }
          break;
          default:
          break;
        }
      }

      activity_counter_.store(data_counter);
    }
  }

  activity_counter_.store(0);
}


uint64_t gnss::Transceiver::current_time() const
{
  // TODO: Needs to be synchronized time with receiver device / GPS
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}


uint64_t gnss::Transceiver::current_systime() const
{
  return timer.st_now();
}


uint32_t gnss::Transceiver::current_ticks() const
{
  // TODO: Implement access to cpu ticks (LKM?)
  return 1337;
}


void gnss::Transceiver::write_gnss_data(std::ofstream& fs, const nmea::RmcData* data) const
{
  fs << "RMC" << ','
     << data->talker_id << ','
     << static_cast<int>(data->utc_time_hour) << ','
     << static_cast<int>(data->utc_time_minute) << ','
     << data->utc_time_second << ','
     << data->degrees_lat << ','
     << data->minutes_lat << ','
     << data->direction_lat << ','
     << data->degrees_long << ','
     << data->minutes_long << ','
     << data->direction_long << ','
     << data->speed_over_ground << ','
     << data->track_angle << ','
     << static_cast<int>(data->date_day) << ','
     << static_cast<int>(data->date_month) << ','
     << static_cast<int>(data->date_year) << ','
     << data->magnetic_variation << ','
     << data->direction_mv << ','
     << data->mode_indicator;
}


void gnss::Transceiver::write_gnss_data(std::ofstream& fs, const nmea::GgaData* data) const
{
  fs << "GGA" << ','
     << data->talker_id << ','
     << static_cast<int>(data->utc_time_hour) << ','
     << static_cast<int>(data->utc_time_minute) << ','
     << data->utc_time_second << ','
     << data->degrees_lat << ','
     << data->minutes_lat << ','
     << data->direction_lat << ','
     << data->degrees_long << ','
     << data->minutes_long << ','
     << data->direction_long << ','
     << static_cast<int>(data->fix_flag) << ','
     << static_cast<int>(data->satellites_used) << ','
     << data->hor_dilution_of_precision << ','
     << data->altitude << ','
     << data->geoidal_separation << ','
     << data->age_of_dgps_data << ','
     << data->reference_station_id;
}


std::vector<std::tuple<nmea::SentenceType, std::string>>
gnss::filter(std::vector<std::string>&& sentences, const Configuration& conf)
{
  std::vector<std::tuple<nmea::SentenceType, std::string>> filtered_sentences;

  for (auto& s : sentences) {
    auto type = nmea::sentence_type(s);
    if ((type == nmea::SentenceType::Rmc && conf.use_msg_rmc) ||
        (type == nmea::SentenceType::Gga && conf.use_msg_gga) ||
        (type == nmea::SentenceType::Gsv && conf.use_msg_gsv) ||
        (type == nmea::SentenceType::Unknown && conf.use_msg_other))
      filtered_sentences.emplace_back(type, std::move(s));
  }

  return filtered_sentences;
}


void gnss::replace_nonascii(gsl::span<char> s, char c)
{
  // ARM char is unsigned by default
  #ifdef __arm__
    std::replace_if(std::begin(s), std::end(s),
        std::bind(std::greater<char>(), std::placeholders::_1, 127), c);
  #else
    std::replace_if(std::begin(s), std::end(s),
        std::bind(std::less<char>(), std::placeholders::_1, 0), c);
  #endif
}
