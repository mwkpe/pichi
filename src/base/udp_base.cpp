#include "udp_base.h"


#include <iostream>
#include <exception>


void base::open_socket(asio::ip::udp::socket& socket)
{
  try {
    socket.open(asio::ip::udp::v4());
    std::cout << "UDP socket opened" << std::endl;
  }
  catch (asio::system_error& e) {
    std::cerr << "Could not open UDP socket:\n" << e.what() << std::endl;
  }
}


void base::close_socket(asio::ip::udp::socket& socket)
{
  try {
    if (socket.is_open()) {
      socket.close();
      std::cout << "UDP socket closed" << std::endl;
    }
  }
  catch (asio::system_error& e) {
    std::cerr << "Error while closing socket:\n" << e.what() << std::endl;
  }
}


auto base::resolve(asio::io_service& ios, const std::string& ip, std::uint16_t port)
    -> asio::ip::udp::endpoint
{
  asio::ip::udp::endpoint ep;

  try {
    asio::ip::udp::resolver resolver{ios};
    asio::ip::udp::resolver::query query{asio::ip::udp::v4(), ip.c_str(),
        std::to_string(port).c_str()};
    ep = *resolver.resolve(query);
    return ep;
  }
  catch (asio::system_error& e) {
    std::cerr << "Could not resolve UDP endpoint:\n" << e.what() << std::endl;
    return asio::ip::udp::endpoint{};
  }

  return ep;
}


bool base::bind_socket(asio::ip::udp::socket& s, const asio::ip::udp::endpoint& ep)
{
  try {
    s.bind(ep);
    std::cout << "Listening on " << ep.address().to_string()
        << ":" << ep.port() << std::endl;
  }
  catch (asio::system_error& e) {
    std::cerr << "Could not bind UDP socket\n" << e.what() << std::endl;
    return false;
  }

  return true;
}
