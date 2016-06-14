#ifndef SERIAL_READER_H
#define SERIAL_READER_H


#include <cstdint>
#include <array>
#include <string>

#include "../ext/gsl-lite.h"
#include <asio.hpp>


namespace serial {


class Reader
{
public:
  Reader() : serial_port_(io_service_) {}
  virtual ~Reader();
  Reader(const Reader&) = delete;
  Reader& operator=(const Reader&) = delete;

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


class OpenPort
{
  public:
    OpenPort(asio::serial_port& sp, const std::string& port, uint32_t rate);
    ~OpenPort() { close_port(); }
    void close_port();
    bool is_open() const { return serial_port_.is_open(); }

  private:
    asio::serial_port& serial_port_;
};


}  // namespace serial


#endif  // SERIAL_READER_H
