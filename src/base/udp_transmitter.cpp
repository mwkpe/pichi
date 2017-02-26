#include "udp_transmitter.h"


#include <iostream>
#include <exception>

#include "udp_base.h"
#include "udp_socket_opener.h"


base::UdpTransmitter::~UdpTransmitter()
{
  close();
}


void base::UdpTransmitter::close()
{
  if (is_open())
    close_socket(socket_);
}


bool base::UdpTransmitter::open(const std::string& ip, std::uint16_t port)
{
  if (!is_open()) {
    UdpSocketOpener socket_opener{socket_};
    if (socket_.is_open()) {
      remote_ep_ = resolve(io_service_, ip.c_str(), port);
      if (remote_ep_ != decltype(remote_ep_){}) {  // Address resolved?
        std::cout << "Sending to " << remote_ep_.address().to_string()
                  << ":" << remote_ep_.port() << std::endl;
        socket_opener.release();  // Pass responsibility to user/destructor
        return true;
      }
    }
  }
  else
    std::cerr << "UDP socket is already open!" << std::endl;

  return false;
}


void base::UdpTransmitter::send(gsl::span<std::uint8_t> data)
{
  socket_.send_to(asio::buffer(data.data(), data.size()), remote_ep_);
}
