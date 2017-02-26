#include "udp_socket_opener.h"


#include <iostream>
#include <exception>

#include "udp_base.h"


base::UdpSocketOpener::UdpSocketOpener(asio::ip::udp::socket& s) : socket_{s}
{
  open_socket(socket_);
}


base::UdpSocketOpener::~UdpSocketOpener()
{
  if (!is_released_)
    close_socket(socket_);
}
