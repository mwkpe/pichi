#ifndef PICHI_H_
#define PICHI_H_


#include <cstdint>
#include <tuple>
#include <string>
#include <deque>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "ext/gsl.h"

#include "configuration.h"
#include "timer.h"
#include "nmea_parser.h"
#include "nmea_reader.h"
#include "gnss_receiver.h"


class Pichi final
{
public:
  explicit Pichi(Configuration&& conf);
  ~Pichi();
  Pichi(const Pichi&) = delete;
  Pichi& operator=(const Pichi&) = delete;

  const Configuration& config() const { return conf_; }
  bool set_config(const Configuration& conf);

  void start_location_transmitter();
  void start_gnss_receiver();
  void start_debugger();

  void stop();

  bool is_active() const { return active_.load(); }
  uint64_t activity_count() const { return activity_counter_.load(); }

private:
  void reset();

  void transmit_location();
  bool parse_location(gnss::LocationPacket* location, const nmea::ReadData& nmea_read);

  void log_gnss_packets();
  void show_nmea_sentences();

  // Member
  Configuration conf_;
  Timer timer_;
  std::atomic<bool> active_{false};
  std::atomic<uint64_t> activity_counter_{0};

  // GNSS receiver
  std::condition_variable nmea_data_ready_;
  std::mutex nmea_data_mutex_;
  std::deque<nmea::ReadData> nmea_data_;
  nmea::Reader nmea_reader_;

  // UDP receiver
  std::condition_variable gnss_data_ready_;
  std::mutex gnss_data_mutex_;
  std::deque<gnss::ReceiveData> gnss_data_;
  gnss::Receiver gnss_receiver_;
};


#endif  // PICHI_H_
