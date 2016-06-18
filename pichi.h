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
#include "gnss_packet.h"
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

  void start_gnss_transmitter();
  void start_gnss_receiver();
  void start_gnss_logger();
  void start_gnss_display();
  void start_debugger();

  void stop();

  bool is_active() const { return active_.load(); }
  uint64_t activity_count() const { return activity_counter_.load(); }

  gnss::LocationPacket current_gnss_location();

private:
  void reset();

  void transmit_gnss_packets();
  void log_gnss_packets();
  void log_gnss_data();
  void update_gnss_data();
  void show_nmea_sentences();

  bool parse_location(gsl::not_null<gnss::LocationPacket*> location,
                      const nmea::ReadData& nmea_read);
  void set_gnss_location(gsl::not_null<const gnss::LocationPacket*> location);

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

  std::mutex gnss_location_mutex_;
  gnss::LocationPacket gnss_location_;
};


#endif  // PICHI_H_
