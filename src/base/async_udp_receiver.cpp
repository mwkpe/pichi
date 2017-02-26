#include "async_udp_receiver.h"


#include <iostream>
#include <exception>

#include "udp_base.h"
#include "udp_socket_opener.h"


base::AsyncUdpReceiver::~AsyncUdpReceiver()
{
  stop();
}


void base::AsyncUdpReceiver::stop()
{
  // Cancel async handlers
  if (is_running())
    socket_.cancel();
}


void base::AsyncUdpReceiver::start_(const std::string& ip, std::uint16_t port)
{
  if (!is_running()) {
    UdpSocketOpener socket_opener{socket_};
    if (socket_.is_open()) {
      auto local_ep = resolve(io_service_, ip.c_str(), port);
      if (bind_socket(socket_, local_ep)) {
        if (io_service_.stopped()) {
          io_service_.reset();
        }
        start_async_receive();  // The io_service needs work queued
        io_service_.run();  // Blocks until all work is done
      }
    }
  }
  else
    std::cerr << "UDP AsyncUdpReceiver is already running!" << std::endl;
}


void base::AsyncUdpReceiver::start_async_receive()
// Starts async receives until AsyncUdpReceiver::stop() is called
{
  socket_.async_receive(asio::buffer(buffer_),
      [this](const asio::error_code& error, std::size_t n) {
        if (!error && n > 0) {
          handle_receive(gsl::as_span(buffer_).first(n));
        }

        if (!error)
          start_async_receive();  // Prevent io_service from stopping
  });
}
