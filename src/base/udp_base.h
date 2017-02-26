#ifndef BASE_UDP_BASE_H_
#define BASE_UDP_BASE_H_


#include <cstdint>
#include <string>

#define ASIO_STANDALONE
#include <asio.hpp>
#undef ASIO_STANDALONE


namespace base {


void open_socket(asio::ip::udp::socket& socket);
void close_socket(asio::ip::udp::socket& socket);
auto resolve(asio::io_service& io_service, const std::string& ip, std::uint16_t port)
    -> asio::ip::udp::endpoint;
bool bind_socket(asio::ip::udp::socket& socket, const asio::ip::udp::endpoint& ep);


}  // namespace base


#endif  // BASE_UDP_BASE_H_
