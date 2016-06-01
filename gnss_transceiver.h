#ifndef GNSS_TRANSCEIVER_H
#define GNSS_TRANSCEIVER_H


/* A tranceiver for GNSS positional data

   Reads data from a GNSS receiver and transmits the data to a remote UDP
   endpoint. Can also act as the remote endpoint and/or log the GNSS data.
*/


#include <cstdint>
#include <string>
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include <asio.hpp>

#include "timer.h"


namespace nmea {
  struct rmc_data;
  struct gga_data;
  struct gsv_data;
}


namespace gnss
{


enum class PacketType : std::uint16_t { GxRMC = 0x10 };

struct PacketHeader
{
  uint16_t data_type;
  uint16_t data_length;
  uint32_t transmit_counter;
  uint64_t transmit_time;
  uint32_t transmit_system_delay;
  uint16_t device_id;
};


class Configuration
{
public:
  Configuration() = default;
  explicit Configuration(const std::string& filename);
  void save_to_file() const;

  uint16_t device_id{1};

  std::string trans_ip{"192.168.0.1"};
  uint16_t trans_port{30001};

  std::string recv_ip{"192.168.0.2"};
  uint16_t recv_port{30001};

  std::string gnss_port{"/dev/ttyS0"};

  bool log_recv{true};

private:
  std::string filename_{};
};


class Transceiver
{
public:
  Transceiver() = default;
  explicit Transceiver(Configuration&& conf);
  ~Transceiver();

  void start_gnss_transmitter();
  void start_gnss_receiver(const std::string& logfilename);
  void start_nmea_logger(const std::string& logfilename);
  void start_gnss_logger(const std::string& logfilename);

  void stop();

  bool is_active() { return active_.load(); }
  uint64_t activity_count() { return activity_counter_.load(); }

  const Configuration& config() { return conf_; }
  void set_config(const Configuration& conf) { conf_ = conf; }

private:
  void reset();  // Reset data and state

  // Provider
  void read_nmea_sentences();
  void receive_gnss_packets(const std::string& logfilename);

  // Consumer
  void transmit_gnss_packets();
  void log_nmea_sentences(const std::string& logfilename);
  void log_gnss_data(const std::string& logfilename);

  // Timing
  uint64_t current_time() const;  // Synchronized reference time
  uint64_t current_systime() const;  // Microsecond system timer
  uint32_t current_ticks() const;  // CPU ticks

  // Logging
  void write_gnss_read(std::ofstream& fs, const nmea::rmc_data* data,
                       uint64_t receive_time, uint64_t system_delay) const;
  void write_gnss_recv(std::ofstream& fs, const PacketHeader* header,
                       const nmea::rmc_data* data, uint64_t receive_time) const;
  void write_gnss_data(std::ofstream& fs, const nmea::rmc_data* data) const;

  Configuration conf_{};
  Timer timer;

  // Thread management
  std::atomic<bool> active_{false};

  // Thread-safe data handling
  std::condition_variable data_cond_;
  std::mutex data_mutex_;

  // Data
  std::queue<std::tuple<uint64_t, uint64_t, std::string>> nmea_sentences_;

  // A counter for transmitted, received or logged packets
  std::atomic<uint64_t> activity_counter_{0};
};


}  // gnss


#endif  // GNSS_TRANSCEIVER_H
