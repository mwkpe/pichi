#include "udp_transceiver.h"


#include <iostream>
#include <exception>


udpt::UdpTransceiver::~UdpTransceiver()
{
  stop();
}


void udpt::UdpTransceiver::stop()
{
  // Cancel async handlers
  if (is_running())
    socket_.cancel();
}


void udpt::UdpTransceiver::start_transmitter_(const std::string& ip, uint16_t port)
{
  if (!is_running()) {
    OpenUdpSocket open_socket(socket_);
    if (open_socket.is_open()) {
      auto remote_ep = resolve(io_service_, ip.c_str(), port);
      if (remote_ep != decltype(remote_ep){}) {
        // TODO: Implement transmit
      }
    }
  }
  else std::cerr << "UDP receiver is already running!" << std::endl;
}


void udpt::UdpTransceiver::start_receiver_(const std::string& ip, uint16_t port)
{
  if (!is_running()) {
    OpenUdpSocket open_socket(socket_);
    if (open_socket.is_open()) {
      auto local_ep = resolve(io_service_, ip.c_str(), port);
      if (bind_socket(socket_, local_ep)) {
        if (io_service_.stopped()) {
          io_service_.reset();
        }
        start_async_receive();  // Service needs work queued or may return
        io_service_.run();
      }
    }
  }
  else std::cerr << "UDP receiver is already running!" << std::endl;
}


void udpt::UdpTransceiver::start_async_receive()
// Starts async receives until UdpTransceiver::stop() is called
{
  socket_.async_receive(asio::buffer(buffer_),
    [this](const asio::error_code& error, std::size_t n)
    {
      if (!error && n > 0) {
        handle_receive(gsl::as_span(buffer_).first(n));
      }

      if (!error)
        start_async_receive();  // Prevent service from stopping
    });
}


auto udpt::resolve(asio::io_service& ios, const std::string& ip, uint16_t port)
  -> asio::ip::udp::endpoint
{
  using asio::ip::udp;
  udp::endpoint ep;

  try {
    udp::resolver resolver{ios};
    udp::resolver::query query{udp::v4(), ip.c_str(), std::to_string(port).c_str()};
    ep = *resolver.resolve(query);
    return ep;
  }
  catch (asio::system_error& e) {
    std::cerr << "Could not resolve UDP endpoint:\n" << e.what() << std::endl;
    return udp::endpoint{};
  }

  return ep;
}


bool udpt::bind_socket(asio::ip::udp::socket& s, const asio::ip::udp::endpoint& ep)
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


udpt::OpenUdpSocket::OpenUdpSocket(asio::ip::udp::socket& s)
  : socket_(s)
{
  try {
    socket_.open(asio::ip::udp::v4());
    std::cout << "UDP socket opened" << std::endl;
  }
  catch (asio::system_error& e) {
    std::cerr << "Could not open UDP socket:\n" << e.what() << std::endl;
  }
}


void udpt::OpenUdpSocket::close_socket()
{
  try {
    if (is_open()) {
      socket_.close();
      std::cout << "UDP socket closed" << std::endl;
    }
  }
  catch (asio::system_error& e) {
    std::cerr << "Error while closing socket:\n" << e.what() << std::endl;
  }
}
