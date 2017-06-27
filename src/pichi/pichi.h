#ifndef PICHI_PICHI_H_
#define PICHI_PICHI_H_


#include <cstdint>
#include <tuple>
#include <string>
#include <deque>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "../ext/gsl.h"

#include "packet.h"
#include "device.h"
#include "receiver.h"
#include "configuration.h"
#include "../util/timer.h"
#include "../nmea/nmea_parser.h"
#include "../nmea/nmea_reader.h"


namespace pichi {


class CsvFile;


class Pichi final
{
public:
  explicit Pichi(Configuration&& conf);
  ~Pichi();
  Pichi(const Pichi&) = delete;
  Pichi& operator=(const Pichi&) = delete;

  const Configuration& config() const { return conf_; }
  bool set_config(const Configuration& conf);

  void start_transmitter();  // Transmit location packets
  void start_receiver();  // Receive location packets from other devices
  void start_logger();  // Log location to file
  void start_device();  // Run without tranceiving or logging
  void start_debug_mode();  // Debugging purposes, e.g. priting NMEA sentences to console

  void stop();

  bool is_active() const { return active_.load(); }
  std::uint64_t activity_count() const { return activity_counter_.load(); }

  std::vector<std::uint16_t> get_device_ids();
  std::tuple<bool, LocationPacket> get_location(std::uint16_t device_id);

private:
  void reset();

  void transmit_packets();
  void receive_packets();
  void log_location();
  void update_location();
  void print_nmea_sentences();

  void handle_receive(const ReceiveData& rx, CsvFile* csv);
  void set_location(std::uint16_t device_id, gsl::not_null<const LocationPacket*> location);

  Configuration conf_;
  util::Timer timer_;
  std::atomic<bool> active_{false};
  std::atomic<std::uint64_t> activity_counter_{0};

  // NMEA serial reader
  std::condition_variable nmea_data_ready_;
  std::mutex nmea_data_mutex_;
  std::deque<NmeaSentence> nmea_data_;
  NmeaReader nmea_reader_;

  // UDP packet receiver
  std::condition_variable rx_data_ready_;
  std::mutex rx_data_mutex_;
  std::deque<ReceiveData> rx_data_;
  Receiver receiver_;

  // The local and remote devices
  std::mutex devices_mutex_;
  std::vector<Device> devices_;
};


}  // namespace pichi


#endif  // PICHI_PICHI_H_
