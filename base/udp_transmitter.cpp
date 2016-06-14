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
      auto remote_ep = resolve(io_service_, ip.c_str(), port);
      if (remote_ep != decltype(remote_ep){}) {  // Address resolved?
        if (connect(socket_, remote_ep)) {
          open_socket.release();  // Pass responsibility to user/destructor
          return true;
        }
      }
    }
  }
  else
    std::cerr << "UDP Transmitter's socket is already open!" << std::endl;

  return false;
}


void udp::Transmitter::send(gsl::span<uint8_t> data)
{
  // TODO: Implemente transmit
}
