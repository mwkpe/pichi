#ifndef SERIAL_READER_H
#define SERIAL_READER_H


#include <cstdint>
#include <array>
#include <string>

#include "gsl-lite.h"
#include <asio.hpp>


class SerialReader
{
public:
  SerialReader() : serial_port_(io_service_) {}
  virtual ~SerialReader();
  SerialReader(const SerialReader&) = delete;
  SerialReader& operator=(const SerialReader&) = delete;

  void start_(const std::string& port, uint32_t rate);
  void stop();
  bool is_running() const { return serial_port_.is_open(); }

protected:
  void start_async_read();
  virtual void handle_read(gsl::span<char> buffer) = 0;

  asio::io_service io_service_;
  asio::serial_port serial_port_;
  std::array<char, 1536> buffer_;
};


class OpenSerialPort
{
  public:
    OpenSerialPort(asio::serial_port& sp, const std::string& port, uint32_t rate);
    ~OpenSerialPort() { close_port(); }
    void close_port();
    bool is_open() const { return serial_port_.is_open(); }

  private:
    asio::serial_port& serial_port_;
};


#endif  // SERIAL_READER_H
