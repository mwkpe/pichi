#ifndef GNSS_TRANSCEIVER_H
#define GNSS_TRANSCEIVER_H


#include <cstdint>
#include <tuple>
#include <string>
#include <deque>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "gsl-lite.h"

#include "configuration.h"
#include "timer.h"
#include "nmea_parser.h"
#include "nmea_reader.h"
#include "udp_transceiver.h"
#include "gnss_packet.h"


namespace gnss {


struct GnssData
{
  GnssData(uint64_t t, uint64_t st, PacketType type, std::vector<uint8_t>&& d)
  : receive_time(t), receive_systime(st), packet_type(type), data(std::move(d)) {}

  uint64_t receive_time;
  uint64_t receive_systime;
  PacketType packet_type;
  std::vector<uint8_t> data;
};


class Transceiver final : public udpt::UdpTransceiver
{
public:
  explicit Transceiver(const Configuration& conf,
                       const Timer& timer,
                       std::condition_variable& data_ready,
                       std::mutex& data_mutex,
                       std::deque<GnssData>& data);
  ~Transceiver();
  Transceiver(const Transceiver&) = delete;
  Transceiver& operator=(const Transceiver&) = delete;

  void start_transmitter();
  void start_receiver();
  uint64_t activity_count() const { return activity_counter_.load(); }

private:
  void handle_receive(gsl::span<uint8_t> buffer) override;

  void reset();
  void log_gnss_data();

  // Member
  std::atomic<bool> active_{false};
  const Configuration& conf_;
  const Timer& timer_;
  std::condition_variable& gnss_data_ready_;
  std::mutex& gnss_data_mutex_;
  std::deque<GnssData>& gnss_data_;

  // A counter for transmitted, received or logged packets
  std::atomic<uint64_t> activity_counter_{0};
};


}  // namespace gnss


#endif  // GNSS_TRANSCEIVER_H
