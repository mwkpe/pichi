#include "async_serial_reader.h"


#include <iostream>
#include <exception>

#include "serial_port_opener.h"


base::AsyncSerialReader::~AsyncSerialReader()
{
  stop();
}


void base::AsyncSerialReader::stop()
{
  if (is_running()) {
    io_service_.stop();  // Cancel async handlers
  }
}


void base::AsyncSerialReader::start_(const std::string& port, uint32_t rate)
{
  if (!is_running()) {
    SerialPortOpener port_opener{serial_port_, port, rate};
    if (serial_port_.is_open()) {
      // A stopped io_service must be reset or run() would return immediately
      if (io_service_.stopped()) {
        io_service_.reset();
      }
      start_async_read();  // The io_service needs work queued
      io_service_.run();  // Blocks until all work is done
    }
  }
  else
    std::cerr << "AsyncSerialReader is already running!" << std::endl;
}


void base::AsyncSerialReader::start_async_read()
// Starts async reads of serial port until stop() is called
{
  serial_port_.async_read_some(asio::buffer(buffer_),
      [this](const asio::error_code& error, std::size_t n) {
        if (!error && n > 0) {
            // Pass a view on the read data to the user
            handle_read(gsl::span<char>(buffer_).first(n));
        }

        if (!error)
          start_async_read();  // Start next and prevent io_service from stopping
  });
}
