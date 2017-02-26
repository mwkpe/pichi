#ifndef SERIAL_PORT_OPENER_H_
#define SERIAL_PORT_OPENER_H_


/* SerialPortOpener is a RAII-style wrapper for opening a serial port */


#include <cstdint>
#include <array>
#include <string>

#include "../ext/gsl.h"

#define ASIO_STANDALONE
#include <asio.hpp>
#undef ASIO_STANDALONE


namespace base {


void close_port(asio::serial_port& serial_port);


class SerialPortOpener final
{
public:
  SerialPortOpener(asio::serial_port& sp, const std::string& port, uint32_t rate);
  ~SerialPortOpener();
  SerialPortOpener(const SerialPortOpener&) = delete;
  SerialPortOpener& operator=(const SerialPortOpener&) = delete;

  void release() { is_released_ = true; }
  bool has_open_port() const { return serial_port_.is_open(); }

private:
  bool is_released_{false};
  asio::serial_port& serial_port_;
};


}  // namespace base


#endif  // SERIAL_PORT_OPENER_H_
