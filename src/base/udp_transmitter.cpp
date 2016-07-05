#include "udp_transmitter.h"
#include "udp_base.h"


#include <iostream>
#include <exception>


udp::Transmitter::~Transmitter()
{
  close();
}


void udp::Transmitter::close()
{
  if (is_open())
    close_socket(socket_);
}


bool udp::Transmitter::open(const std::string& ip, uint16_t port)
{
  if (!is_open()) {
    OpenSocket open_socket(socket_);
    if (open_socket.is_open()) {
      remote_ep_ = resolve(io_service_, ip.c_str(), port);
      if (remote_ep_ != decltype(remote_ep_){}) {  // Address resolved?
        std::cout << "Sending to " << remote_ep_.address().to_string()
                  << ":" << remote_ep_.port() << std::endl;
        open_socket.release();  // Pass responsibility to user/destructor
        return true;
      }
    }
  }
  else
    std::cerr << "UDP Transmitter's socket is already open!" << std::endl;

  return false;
}


void udp::Transmitter::send(gsl::span<uint8_t> data)
{
  socket_.send_to(asio::buffer(data.data(), data.size()), remote_ep_);
}
