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

#include "pichi_device.h"
#include "configuration.h"
#include "timer.h"
#include "nmea/parser.h"
#include "nmea/reader.h"
#include "gnss/packet.h"
#include "gnss/receiver.h"


class Pichi final
{
public:
  explicit Pichi(Configuration&& conf);
  ~Pichi();
  Pichi(const Pichi&) = delete;
  Pichi& operator=(const Pichi&) = delete;

  static const uint16_t local_device_id{0};

  const Configuration& config() const { return conf_; }
  bool set_config(const Configuration& conf);

  void start_gnss_transmitter();
  void start_gnss_receiver();
  void start_location_logger();
  void start_location_display();
  void start_debugger();

  void stop();

  bool is_active() const { return active_.load(); }
  uint64_t activity_count() const { return activity_counter_.load(); }

  std::tuple<bool, gnss::LocationPacket> gnss_location(uint16_t device_id);
  std::vector<uint16_t> new_device_ids();

private:
  void reset();

  void transmit_gnss_packets();
  void receive_gnss_packets();
  void log_position();
  void update_position();
  void show_nmea_sentences();

  bool parse_location(
      gsl::not_null<gnss::LocationPacket*> location,
      const nmea::ReadData& nmea_read);
  bool parse_location(
      gsl::not_null<gnss::LocationPacket*> location,
      const std::string& nmea_sentence,
      nmea::RmcData& rmc_data);
  
  void set_location(
      gsl::not_null<gnss::LocationPacket*> location,
      const nmea::RmcData& rmc_data);
  void set_device_location(
      uint16_t device_id,
      gsl::not_null<const gnss::LocationPacket*> location);

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

  // The local and remote devices
  std::mutex devices_mutex_;
  std::vector<pichi::Device> devices_;
  std::vector<uint16_t> new_device_ids_;
};


#endif  // PICHI_H_
