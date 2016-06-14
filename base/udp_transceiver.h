#ifndef UDP_TRANSCEIVER_H
#define UDP_TRANSCEIVER_H


#include <cstdint>
#include <array>
#include <string>

#include "../ext/gsl-lite.h"
#include <asio.hpp>


namespace udp {


class Transceiver
{
public:
  Transceiver() : socket_(io_service_) {}
  virtual ~Transceiver();
  Transceiver(const Transceiver&) = delete;
  Transceiver& operator=(const Transceiver&) = delete;

  void start_transmitter_(const std::string& ip, uint16_t port);
  void start_receiver_(const std::string& ip, uint16_t port);
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


auto resolve(asio::io_service& io_service, const std::string& ip, uint16_t port)
  -> asio::ip::udp::endpoint;
bool bind_socket(asio::ip::udp::socket& socket, const asio::ip::udp::endpoint& ep);


class OpenSocket
{
  public:
    OpenSocket(asio::ip::udp::socket& s);
    ~OpenSocket() { close_socket(); }
    void close_socket();
    bool is_open() const { return socket_.is_open(); }

  private:
    asio::ip::udp::socket& socket_;
};


}  // namespace udp


#endif  // UDP_TRANSCEIVER_H
