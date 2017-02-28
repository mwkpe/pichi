#ifndef PICHI_RECEIVER_H_
#define PICHI_RECEIVER_H_


#include <cstdint>
#include <vector>
#include <tuple>
#include <string>
#include <deque>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "../ext/gsl.h"

#include "packet.h"
#include "../base/async_udp_receiver.h"
#include "../util/timer.h"


namespace pichi {


struct ReceiveData
{
  util::TimePoint receive_time;
  PacketHeader header;
  std::vector<std::uint8_t> data;
};


class Receiver final : public base::AsyncUdpReceiver
{
public:
  Receiver(const util::Timer& timer, std::condition_variable& data_ready, std::mutex& data_mutex,
      std::deque<ReceiveData>& data);
  ~Receiver();
  Receiver(const Receiver&) = delete;
  Receiver& operator=(const Receiver&) = delete;

  void start(const std::string& ip, std::uint16_t port);
  std::uint64_t activity_count() const { return activity_counter_.load(); }

private:
  void handle_receive(gsl::span<std::uint8_t> buffer) override;
  void reset();

  // Member
  const util::Timer& timer_;
  std::condition_variable& data_ready_;
  std::mutex& data_mutex_;
  std::deque<ReceiveData>& data_;

  // A counter for received packets
  std::atomic<std::uint64_t> activity_counter_{0};
};


}  // namespace pichi


#endif  // PICHI_RECEIVER_H_
