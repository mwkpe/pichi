#include "serial_reader.h"


#include <iostream>
#include <exception>


SerialReader::~SerialReader()
{
  stop();
}


void SerialReader::stop()
{
  // Cancel async handlers
  if (is_running())
    serial_port_.cancel();
}


void SerialReader::start_(const std::string& port, uint32_t rate)
{
  if (!is_running()) {
    OpenSerialPort osp(serial_port_, port, rate);
    if (serial_port_.is_open()) {
      // A stopped io_service must be reset or run() would return immediately
      if (io_service_.stopped()) {
        io_service_.reset();
      }
      start_async_read();  // The io_service needs work or may return
      io_service_.run();  // Blocks until all work is done
    }
  }
  else std::cerr << "Serial port reader is already running!" << std::endl;
}


void SerialReader::start_async_read()
// Starts async reads of serial port until SerialReader::stop() is called
{
  serial_port_.async_read_some(asio::buffer(buffer_),
    [this](const asio::error_code& error, std::size_t n)
    {
      if (!error && n > 0) {
        // Pass a view on the read data to the user-defined handler
        handle_read(gsl::as_span(buffer_).first(n));
      }

      if (!error)
        start_async_read();  // Prevent io_service from stopping
  });
}


OpenSerialPort::OpenSerialPort(asio::serial_port& sp,
                               const std::string& port, uint32_t rate)
  : serial_port_(sp)
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


void OpenSerialPort::close_port()
{
  if (is_open()) {
    serial_port_.close();
    std::cout << "Serial port closed" << std::endl;
  }
}