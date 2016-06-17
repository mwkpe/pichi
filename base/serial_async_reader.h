#ifndef SERIAL_ASYNC_READER_H_
#define SERIAL_ASYNC_READER_H_


#include <asio.hpp>

#include <cstdint>
#include <array>
#include <string>

#include "../ext/gsl.h"


namespace serial {


class AsyncReader
{
public:
  AsyncReader() : serial_port_(io_service_) {}
  virtual ~AsyncReader();
  AsyncReader(const AsyncReader&) = delete;
  AsyncReader& operator=(const AsyncReader&) = delete;

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


}  // namespace serial


#endif  // SERIAL_ASYNC_READER_H_
