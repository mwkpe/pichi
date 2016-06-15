#ifndef UDP_TRANSMITTER_H
#define UDP_TRANSMITTER_H


#include <cstdint>
#include <array>
#include <string>

#include "../ext/gsl-lite.h"
#include <asio.hpp>


namespace udp {


class Transmitter final
{
public:
  Transmitter() : socket_(io_service_) {}
  virtual ~Transmitter();
  Transmitter(const Transmitter&) = delete;
  Transmitter& operator=(const Transmitter&) = delete;

  bool open(const std::string& ip, uint16_t port);
  void close();
  bool is_open() const { return socket_.is_open(); }
  void send(gsl::span<uint8_t> data);

private:
  asio::io_service io_service_;
  asio::ip::udp::socket socket_;
  asio::ip::udp::endpoint remote_ep_;
};


}  // namespace udp


#endif  // UDP_TRANSMITTER_H
