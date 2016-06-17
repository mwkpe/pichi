#ifndef SERIAL_BASE_H_
#define SERIAL_BASE_H_


#include <asio.hpp>

#include <cstdint>
#include <array>
#include <string>

#include "../ext/gsl-lite.h"


namespace serial {


class OpenPort final
{
  public:
    OpenPort(asio::serial_port& sp, const std::string& port, uint32_t rate);
    ~OpenPort();
    OpenPort(const OpenPort&) = delete;
    OpenPort& operator=(const OpenPort&) = delete;

    void release() { is_released_ = true; }
    bool is_open() const { return serial_port_.is_open(); }

  private:
    bool is_released_{false};
    asio::serial_port& serial_port_;
};


void close_port(asio::serial_port& serial_port);


}  // namespace serial


#endif  // SERIAL_BASE_H_
