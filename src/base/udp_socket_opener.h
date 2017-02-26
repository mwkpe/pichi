#ifndef UDP_SOCKET_OPENER_H_
#define UDP_SOCKET_OPENER_H_


#include <cstdint>
#include <array>
#include <string>

#include "../ext/gsl.h"

#define ASIO_STANDALONE
#include <asio.hpp>
#undef ASIO_STANDALONE


namespace base {


class UdpSocketOpener final
{
public:
  explicit UdpSocketOpener(asio::ip::udp::socket& s);
  ~UdpSocketOpener();
  UdpSocketOpener(const UdpSocketOpener&) = delete;
  UdpSocketOpener& operator=(const UdpSocketOpener&) = delete;

  void release() { is_released_ = true; }
  bool has_open_socket() const { return socket_.is_open(); }

private:
  bool is_released_{false};
  asio::ip::udp::socket& socket_;
};


}  // namespace base


#endif  // UDP_SOCKET_OPENER_H_
