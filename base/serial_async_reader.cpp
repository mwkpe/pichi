#include "serial_async_reader.h"
#include "serial_base.h"


#include <iostream>
#include <exception>


serial::AsyncReader::~AsyncReader()
{
  stop();
}


void serial::AsyncReader::stop()
{
  // Cancel async handlers
  if (is_running())
    serial_port_.cancel();
}


void serial::AsyncReader::start_(const std::string& port, uint32_t rate)
{
  if (!is_running()) {
    OpenPort open_port(serial_port_, port, rate);
    if (open_port.is_open()) {
      // A stopped io_service must be reset or run() would return immediately
      if (io_service_.stopped()) {
        io_service_.reset();
      }
      start_async_read();  // The io_service needs work queued
      io_service_.run();  // Blocks until all work is done
    }
  }
  else std::cerr << "Serial port AsyncReader is already running!" << std::endl;
}


void serial::AsyncReader::start_async_read()
// Starts async reads of serial port until stop() is called
{
  serial_port_.async_read_some(asio::buffer(buffer_),
    [this](const asio::error_code& error, std::size_t n)
    {
      if (!error && n > 0) {
        // Pass a view on the read data to the user
        handle_read(gsl::as_span(buffer_).first(n));
      }

      if (!error)
        start_async_read();  // Start next and prevent io_service from stopping
  });
}
