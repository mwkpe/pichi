#ifndef GNSS_RECEIVER_H_
#define GNSS_RECEIVER_H_


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
#include "../base/udp_async_receiver.h"
#include "../timer.h"


namespace gnss {


struct ReceiveData
{
  ReceiveData() = default;
  ReceiveData(
      uint64_t time, uint64_t systime,
      gnss::PacketHeader header, std::vector<uint8_t>&& data)
  : time{time}, systime{systime}, header(header), data(std::move(data)) {}

  uint64_t time;
  uint64_t systime;
  PacketHeader header;
  std::vector<uint8_t> data;
};


class Receiver final : public udp::AsyncReceiver
{
public:
  explicit Receiver(const Timer& timer,
                    std::condition_variable& data_ready,
                    std::mutex& data_mutex,
                    std::deque<ReceiveData>& data);
  ~Receiver();
  Receiver(const Receiver&) = delete;
  Receiver& operator=(const Receiver&) = delete;

  void start(const std::string& ip, uint16_t port);
  uint64_t activity_count() const { return activity_counter_.load(); }

private:
  void handle_receive(gsl::span<uint8_t> buffer) override;
  void reset();

  // Member
  const Timer& timer_;
  std::condition_variable& data_ready_;
  std::mutex& data_mutex_;
  std::deque<ReceiveData>& data_;

  // A counter for received packets
  std::atomic<uint64_t> activity_counter_{0};
};


}  // namespace gnss


#endif  // GNSS_RECEIVER_H_
