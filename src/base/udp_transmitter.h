#ifndef UDP_TRANSMITTER_H_
#define UDP_TRANSMITTER_H_


#include <asio.hpp>

#include <cstdint>
#include <array>
#include <string>

#include "../ext/gsl.h"


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


#endif  // UDP_TRANSMITTER_H_
