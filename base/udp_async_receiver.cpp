#include "udp_async_receiver.h"
#include "udp_base.h"


#include <iostream>
#include <exception>


udp::AsyncReceiver::~AsyncReceiver()
{
  stop();
}


void udp::AsyncReceiver::stop()
{
  // Cancel async handlers
  if (is_running())
    socket_.cancel();
}


void udp::AsyncReceiver::start_(const std::string& ip, uint16_t port)
{
  if (!is_running()) {
    OpenSocket open_socket(socket_);
    if (open_socket.is_open()) {
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
    std::cerr << "UDP AsyncReceiver is already running!" << std::endl;
}


void udp::AsyncReceiver::start_async_receive()
// Starts async receives until AsyncReceiver::stop() is called
{
  socket_.async_receive(asio::buffer(buffer_),
    [this](const asio::error_code& error, std::size_t n)
    {
      if (!error && n > 0) {
        handle_receive(gsl::as_span(buffer_).first(n));
      }

      if (!error)
        start_async_receive();  // Prevent io_service from stopping
    });
}
