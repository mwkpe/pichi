#ifndef ASYNC_SERIAL_READER_H_
#define ASYNC_SERIAL_READER_H_


/* AsyncSerialReader is a base class for implementing an asynchronous serial reader. A derived
   class simply has to implement handle_read which gets passed a view of the received data.
   The function start_ will block and should be called in its own thread. */


#include <cstdint>
#include <array>
#include <string>

#include "../ext/gsl.h"

#define ASIO_STANDALONE
#include <asio.hpp>
#undef ASIO_STANDALONE


namespace base {


class AsyncSerialReader
{
public:
  static constexpr size_t READ_BUFFER_SIZE = 1536;

  AsyncSerialReader() : serial_port_{io_service_} {}
  virtual ~AsyncSerialReader();
  AsyncSerialReader(const AsyncSerialReader&) = delete;
  AsyncSerialReader& operator=(const AsyncSerialReader&) = delete;

  void start_(const std::string& port, uint32_t rate);
  void stop();
  bool is_running() const { return serial_port_.is_open(); }

protected:
  void start_async_read();
  virtual void handle_read(gsl::span<char> received_data) = 0;

  asio::io_service io_service_;
  asio::serial_port serial_port_;
  std::array<char, READ_BUFFER_SIZE> buffer_;
};


}  // namespace base


#endif  // ASYNC_SERIAL_READER_H_
