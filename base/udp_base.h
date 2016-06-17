#ifndef UDP_BASE_H_
#define UDP_BASE_H_


#include <asio.hpp>

#include <cstdint>
#include <array>
#include <string>

#include "../ext/gsl-lite.h"


namespace udp {


class OpenSocket final
{
  public:
    explicit OpenSocket(asio::ip::udp::socket& s);
    ~OpenSocket();
    OpenSocket(const OpenSocket&) = delete;
    OpenSocket& operator=(const OpenSocket&) = delete;

    void release() { is_released_ = true; }
    bool is_open() const { return socket_.is_open(); }

  private:
    bool is_released_{false};
    asio::ip::udp::socket& socket_;
};


auto resolve(asio::io_service& io_service, const std::string& ip, uint16_t port)
  -> asio::ip::udp::endpoint;
bool bind_socket(asio::ip::udp::socket& socket, const asio::ip::udp::endpoint& ep);
void close_socket(asio::ip::udp::socket& socket);


}  // namespace udp


#endif  // UDP_BASE_H_
