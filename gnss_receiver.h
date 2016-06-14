#ifndef GNSS_RECEIVER_H
#define GNSS_RECEIVER_H


#include <cstdint>
#include <tuple>
#include <string>
#include <deque>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "ext/gsl-lite.h"

#include "base/udp_async_receiver.h"
#include "configuration.h"
#include "timer.h"
#include "nmea_parser.h"
#include "nmea_reader.h"
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


class Receiver final : public udp::AsyncReceiver
{
public:
  explicit Receiver(const Configuration& conf,
                    const Timer& timer,
                    std::condition_variable& data_ready,
                    std::mutex& data_mutex,
                    std::deque<GnssData>& data);
  ~Receiver();
  Receiver(const Receiver&) = delete;
  Receiver& operator=(const Receiver&) = delete;

  void start();
  uint64_t activity_count() const { return activity_counter_.load(); }

private:
  void handle_receive(gsl::span<uint8_t> buffer) override;
  void reset();

  // Member
  const Configuration& conf_;
  const Timer& timer_;
  std::condition_variable& gnss_data_ready_;
  std::mutex& gnss_data_mutex_;
  std::deque<GnssData>& gnss_data_;

  // A counter for received packets
  std::atomic<uint64_t> activity_counter_{0};
};


}  // namespace gnss


#endif  // GNSS_RECEIVER_H
