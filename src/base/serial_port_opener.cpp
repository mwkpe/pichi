#include "serial_port_opener.h"


#include <iostream>
#include <exception>


void base::close_port(asio::serial_port& serial_port)
{
  if (serial_port.is_open()) {
    serial_port.close();
    std::cout << "Serial port closed" << std::endl;
  }
}


base::SerialPortOpener::SerialPortOpener(asio::serial_port& sp, const std::string& port,
    uint32_t rate) : serial_port_{sp}
{
  try {
    using base = asio::serial_port_base;
    serial_port_.open(port);
    serial_port_.set_option(base::baud_rate(rate));
    serial_port_.set_option(base::character_size(8));
    serial_port_.set_option(base::stop_bits(base::stop_bits::one));
    serial_port_.set_option(base::parity(base::parity::none));
    serial_port_.set_option(base::flow_control(base::flow_control::none));
    std::cout << "Serial port " << port << " opened" << std::endl;
  }
  catch (std::exception& e) {
    std::cerr << "Could not open serial port " << port
              << ":\n" << e.what() << std::endl;
  }
}


base::SerialPortOpener::~SerialPortOpener()
{
  if (!is_released_)
    close_port(serial_port_);
}
