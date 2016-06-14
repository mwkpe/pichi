#ifndef UDP_ASYNC_RECEIVER_H
#define UDP_ASYNC_RECEIVER_H


#include <cstdint>
#include <array>
#include <string>

#include "../ext/gsl-lite.h"
#include <asio.hpp>


namespace udp {


class AsyncReceiver
{
public:
  AsyncReceiver() : socket_(io_service_) {}
  virtual ~AsyncReceiver();
  AsyncReceiver(const AsyncReceiver&) = delete;
  AsyncReceiver& operator=(const AsyncReceiver&) = delete;

  void start_(const std::string& ip, uint16_t port);
  void stop();
  bool is_running() const { return socket_.is_open(); }

protected:
  virtual void handle_receive(gsl::span<uint8_t> buffer) = 0;

private:
  void start_async_receive();

  asio::io_service io_service_;
  asio::ip::udp::socket socket_;
  std::array<uint8_t, 1536> buffer_;
};


}  // namespace udp


#endif  // UDP_ASYNC_RECEIVER_H
