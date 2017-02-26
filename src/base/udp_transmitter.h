#ifndef UDP_TRANSMITTER_H_
#define UDP_TRANSMITTER_H_


#include <cstdint>
#include <array>
#include <string>

#include "../ext/gsl.h"

#define ASIO_STANDALONE
#include <asio.hpp>
#undef ASIO_STANDALONE


namespace base {


class UdpTransmitter final
{
public:
  UdpTransmitter() : socket_(io_service_) {}
  virtual ~UdpTransmitter();
  UdpTransmitter(const UdpTransmitter&) = delete;
  UdpTransmitter& operator=(const UdpTransmitter&) = delete;

  bool open(const std::string& ip, std::uint16_t port);
  void close();
  bool is_open() const { return socket_.is_open(); }
  void send(gsl::span<std::uint8_t> data);

private:
  asio::io_service io_service_;
  asio::ip::udp::socket socket_;
  asio::ip::udp::endpoint remote_ep_;
};


}  // namespace base


#endif  // UDP_TRANSMITTER_H_
