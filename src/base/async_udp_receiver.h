#ifndef ASYNC_UDP_RECEIVER_H_
#define ASYNC_UDP_RECEIVER_H_


/* AsyncUdpReceiver is a base class for implementing an asynchronous UDP receiver. A derived
   class simply has to implement handle_receive which gets passed a view of the received data.
   The function start_ will block and should be called in its own thread. */


#include <cstdint>
#include <array>
#include <string>

#include "../ext/gsl.h"

#define ASIO_STANDALONE
#include <asio.hpp>
#undef ASIO_STANDALONE


namespace base {


class AsyncUdpReceiver
{
public:
  AsyncUdpReceiver() : socket_{io_service_} {}
  virtual ~AsyncUdpReceiver();
  AsyncUdpReceiver(const AsyncUdpReceiver&) = delete;
  AsyncUdpReceiver& operator=(const AsyncUdpReceiver&) = delete;

  void start_(const std::string& ip, std::uint16_t port);
  void stop();
  bool is_running() const { return socket_.is_open(); }

protected:
  virtual void handle_receive(gsl::span<std::uint8_t> buffer) = 0;

private:
  void start_async_receive();

  asio::io_service io_service_;
  asio::ip::udp::socket socket_;
  std::array<std::uint8_t, 1536> buffer_;
};


}  // namespace base


#endif  // ASYNC_UDP_RECEIVER_H_
