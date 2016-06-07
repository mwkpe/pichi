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
#include <exception>

#include <asio.hpp>

#include "nmea_parser.h"


gnss::Transceiver::Transceiver(Configuration&& conf)
  : conf_{std::move(conf)}, serial_port_{serial_service_}, udp_socket_{udp_service_}
{
  if (!timer.st_init())
    std::cerr << "Values relying on the microsecond system timer will be zero." << std::endl;
}


gnss::Transceiver::~Transceiver()
{
  if (is_active())
    stop();
  conf_.save_to_file();

  // Give detached threads some time to properly finish
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
}


bool gnss::Transceiver::set_config(const Configuration& conf)
{
  if (is_active()) {
    std::cerr << "Can't set configuration while transceiver is running!" << std::endl;
    return false;
  }

  conf_ = conf;
  std::cout << "Configuration updated!" << std::endl;
  return true;
}


void gnss::Transceiver::start_gnss_transmitter()
{
  if (!is_active()) {
    reset();
    active_.store(true);

    std::thread consumer{&gnss::Transceiver::transmit_gnss_packets, this};
    consumer.detach();

    std::thread provider{&gnss::Transceiver::read_nmea_sentences, this};
    provider.detach();
  }
  else std::cerr << "Transceiver is already running!" << std::endl;
}


void gnss::Transceiver::start_gnss_receiver(const std::string& logfilename)
{
  if (!is_active()) {
    reset();
    active_.store(true);
    
    std::thread t{&gnss::Transceiver::receive_gnss_packets, this, logfilename};
    t.detach();
  }
  else std::cerr << "Transceiver is already running!" << std::endl;
}


void gnss::Transceiver::start_nmea_logger(const std::string& logfilename)
{
  if (!is_active()) {
    reset();
    active_.store(true);

    std::thread consumer{&gnss::Transceiver::log_nmea_sentences, this, logfilename};
    consumer.detach();

    std::thread provider{&gnss::Transceiver::read_nmea_sentences, this};
    provider.detach();
  }
  else std::cerr << "Transceiver is already running!" << std::endl;
}


void gnss::Transceiver::start_gnss_logger(const std::string& logfilename)
{
  if (!is_active()) {
    reset();
    active_.store(true);

    std::thread consumer{&gnss::Transceiver::log_gnss_data, this, logfilename};
    consumer.detach();

    std::thread provider{&gnss::Transceiver::read_nmea_sentences, this};
    provider.detach();
  }
  else std::cerr << "Transceiver is already running!" << std::endl;
}


void gnss::Transceiver::read_nmea_sentences()
{
  if (open_serial(conf_.gnss_port)) {
    if (serial_service_.stopped()) {
      serial_service_.reset();
    }

    // The service needs something to do or it may return immediately
    read_serial();  // Start an async read
    serial_service_.run();  // Start processing
  }
}


void gnss::Transceiver::stop()
{
  // Cancel async handlers
  if (serial_port_.is_open())
    serial_port_.cancel();
  if (udp_socket_.is_open())
    udp_socket_.cancel();

  // Stop detached threads
  active_.store(false);
  data_cond_.notify_all();
}


void gnss::Transceiver::reset()
{
  activity_counter_.store(0);
  std::lock_guard<std::mutex> lk(data_mutex_);
  decltype(nmea_sentences_){}.swap(nmea_sentences_);
}


void gnss::Transceiver::receive_gnss_packets(const std::string& logfilename)
{
  if (open_socket()) {
    auto local_ep = resolve(conf_.recv_ip.c_str(), conf_.recv_port);
    if (bind_socket(local_ep)) {
      std::cout << "Listening on " << local_ep.address().to_string()
                                   << " (" << local_ep.port() << ")" << std::endl;

      std::ofstream logfile;
      if (conf_.log_recv) {
        logfile.open(logfilename);
        if (!logfile.is_open()) {
          std::cerr << "Error opening logfile: " << logfilename << std::endl;
        }
      }

      if (udp_service_.stopped()) {
        udp_service_.reset();
      }

      // The service needs something to do or it may return immediately
      receive(logfile);  // Start an async receive
      udp_service_.run();  // Start processing
    }
    else {
      close_socket();
    }
  }
}


void gnss::Transceiver::transmit_gnss_packets()
{
  if (open_socket()) {
    auto remote_ep = resolve(conf_.trans_ip.c_str(), conf_.trans_port);
    if (remote_ep == decltype(remote_ep){}) {
      close_socket();
      stop();
    }
    else {
      std::cout << "Transmitting to " << remote_ep.address().to_string()
                                      << " (" << remote_ep.port() << ")" << std::endl;
      transmit_to(remote_ep);  // Transmit is done with synchronous asio functions
      close_socket();
    }
  }
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


bool gnss::Transceiver::open_serial(const std::string& port)
{
  try {
    asio::error_code ec;
    serial_port_.open(port, ec);
    if (ec) {
      std::cerr << "Could not open serial port " << port
                << ":\n" << ec.message() << std::endl;
      return false;
    }
    else {
      using spb = asio::serial_port_base;
      serial_port_.set_option(spb::baud_rate(115200));
      serial_port_.set_option(spb::character_size(8));
      serial_port_.set_option(spb::stop_bits(asio::serial_port_base::stop_bits::one));
      serial_port_.set_option(spb::parity(asio::serial_port_base::parity::none));
      serial_port_.set_option(spb::flow_control(asio::serial_port_base::flow_control::none));
      std::cout << "Opened serial port " << port << std::endl;
    }
  }
  catch (std::exception& e) {
    std::cerr << "Could not open serial port " << port
              << ":\n" << e.what() << std::endl;
    return false;
  }

  return true;
}


void gnss::Transceiver::read_serial()
{
  serial_port_.async_read_some(asio::buffer(serial_buffer_),
  [this](const asio::error_code& error, std::size_t bytes_transferred)
  {
    auto recv_st = current_systime();
    auto recv_time = current_time();

    if (!error && bytes_transferred > 0) {
      // Prevent crash due to spirit isascii assert when garbage was received
      replace_nonascii(gsl::as_span(serial_buffer_).first(bytes_transferred), '0');

      // Look for sentences in the buffer
      // Note: Using read_some returned a block of sentences, whereas async_read_some seems
      //       to call this handler for each sentence separately, so this may not be necessary
      //       anymore but is kept for now to make sure no sentences go missing.
      std::vector<std::string> sentences;
      auto pos = std::begin(serial_buffer_);
      auto last = std::begin(serial_buffer_);
      std::advance(last, bytes_transferred);
      auto sentence_start = pos;
      while (pos != last) {
        pos = std::find_if(pos, last, [](char c) { return c == '\r'; });
        if (pos != last) {
          sentences.push_back({sentence_start, pos});
          std::advance(pos, 1);  // Skip carriage return
          if (pos != last)
            std::advance(pos, 1);  // Skip newline
          sentence_start = pos;
        }
      }

      // Push data and notifiy other threads
      if (!sentences.empty()) {
        auto filtered_sentences = filter(std::move(sentences), conf_);
        std::lock_guard<std::mutex> lk(data_mutex_);
        for (auto& t : filtered_sentences) {
          nmea_sentences_.emplace(recv_st, recv_time,
                                  std::get<0>(t), std::move(std::get<1>(t)));
        }
        data_cond_.notify_one();
      }
    }
    else if (error == asio::error::operation_aborted) {
      if (serial_port_.is_open()) {
        serial_port_.close();
        std::cerr << "Port " << conf_.gnss_port << " closed" << std::endl;
      }
      // Let all already queued handlers finish, the io_service will stop automatically
    }
    
    if (!error)
      read_serial();  // Start next async read and prevent the io_service from stopping
  });
}


bool gnss::Transceiver::open_socket()
{
  try {
    udp_socket_.open(asio::ip::udp::v4());
    std::cout << "UDP socket opened" << std::endl;
  }
  catch (asio::system_error& e) {
    std::cerr << "Could not open UDP socket:\n" << e.what() << std::endl;
    active_.store(false);
    return false;
  }

  return true;
}


void gnss::Transceiver::close_socket()
{
  try {
    udp_socket_.close();
  }
  catch (asio::system_error& e) {
    std::cerr << "An error occured while closing the UDP socket:\n"
              << e.what() << std::endl;
  }

  // Even if the function indicates an error, the underlying descriptor is closed
  std::cout << "UDP socket closed" << std::endl;
}


bool gnss::Transceiver::bind_socket(const asio::ip::udp::endpoint& ep)
{
  try {
    udp_socket_.bind(ep);
  }
  catch (asio::system_error& e) {
    std::cerr << "Could not bind UDP socket\n" << e.what() << std::endl;
    return false;
  }

  return true;
}


asio::ip::udp::endpoint gnss::Transceiver::resolve(const std::string& ip, uint16_t port)
{
  using asio::ip::udp;
  udp::endpoint ep;

  try {
    udp::resolver resolver{udp_service_};
    udp::resolver::query query{udp::v4(), ip.c_str(), std::to_string(port).c_str()};
    ep = *resolver.resolve(query);
    return ep;
  }
  catch (asio::system_error& e) {
    std::cerr << "Could not resolve UDP endpoint:\n" << e.what() << std::endl;
    return asio::ip::udp::endpoint{};
  }

  return ep;
}


void gnss::Transceiver::transmit_to(const asio::ip::udp::endpoint& remote_ep)
{
  std::array<char, rmc_packet_size> rmc_buffer{0};
  auto* rmc_header = reinterpret_cast<PacketHeader*>(rmc_buffer.data());
  rmc_header->data_type = static_cast<uint16_t>(PacketType::GxRmc);
  rmc_header->data_length = static_cast<uint16_t>(rmc_data_size);
  rmc_header->device_id = conf_.device_id;
  auto* rmc_data = reinterpret_cast<nmea::RmcData*>(rmc_buffer.data() + header_size);

  std::array<char, gga_packet_size> gga_buffer{0};
  auto* gga_header = reinterpret_cast<PacketHeader*>(gga_buffer.data());
  gga_header->data_type = static_cast<uint16_t>(PacketType::GxGga);
  gga_header->data_length = static_cast<uint16_t>(gga_data_size);
  gga_header->device_id = conf_.device_id;
  auto* gga_data = reinterpret_cast<nmea::GgaData*>(gga_buffer.data() + header_size);

  uint64_t packet_counter = 0;

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
              udp_socket_.send_to(asio::buffer(rmc_buffer), remote_ep);
              packet_counter++;
            }
          }
          break;
          case nmea::SentenceType::Gga: {
            if (nmea::parse(s, gga_data, &crc) && nmea::comp_checksum(s, crc)) {
              gga_header->transmit_counter++;
              gga_header->transmit_time = current_time();
              gga_header->transmit_system_delay = static_cast<uint32_t>(current_systime() - recv_st);
              udp_socket_.send_to(asio::buffer(gga_buffer), remote_ep);
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
}


void gnss::Transceiver::receive(std::ofstream& logfile)
{
  udp_socket_.async_receive(asio::buffer(udp_buffer_),
  [this, &logfile](const asio::error_code& error, std::size_t bytes_transferred)
  {
    auto receive_time = current_time();

    if (!error && bytes_transferred >= header_size) {
      auto* header = reinterpret_cast<PacketHeader*>(udp_buffer_.data());
      
      switch (static_cast<PacketType>(header->data_type)) {
        case PacketType::GxRmc: {
          if (bytes_transferred >= rmc_packet_size && header->data_length == rmc_data_size) {
            auto* data = reinterpret_cast<nmea::RmcData*>(udp_buffer_.data() + header_size);
            if (logfile.is_open()) {
              write_gnss_recv(logfile, header, data, receive_time);
            }
            activity_counter_.fetch_add(1);
          }
        }
        break;
        case PacketType::GxGga: {
          if (bytes_transferred >= gga_packet_size && header->data_length == gga_data_size) {
            auto* data = reinterpret_cast<nmea::GgaData*>(udp_buffer_.data() + header_size);
            if (logfile.is_open()) {
              write_gnss_recv(logfile, header, data, receive_time);
            }
            activity_counter_.fetch_add(1);
          }
        }
        break;
        default:
        break;
      }
    }
    else if (error == asio::error::operation_aborted) {  // User cancled
      if (udp_socket_.is_open()) {
        close_socket();
      }
      // The io_service will stop once all handlers are done
    }

    if (!error)
      receive(logfile);  // Start next async receive
  });
}


uint64_t gnss::Transceiver::current_time() const
{
  // Needs to be synchronized with other device for transmit delay timing
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
