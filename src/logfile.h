#ifndef LOGFILE_H_
#define LOGFILE_H_


#include <string>
#include <fstream>

#include "ext/gsl.h"
#include "gnss/packet.h"


namespace logging {


class Logfile
{
public:
  Logfile() = default;
  explicit Logfile(const std::string& filename);
  ~Logfile();
  Logfile(const Logfile&) = delete;
  Logfile& operator=(const Logfile&) = delete;

  bool open(const std::string& filename);
  bool is_open() const { return fs_.is_open(); }

  template<typename T> void write(gsl::not_null<const gnss::PacketHeader*> header,
                                  const T* data,
                                  uint64_t receive_time);
  template<typename T> void write(const T* data,
                                  uint16_t packet_type,
                                  uint16_t device_id,
                                  uint64_t receive_time);
  void write(gsl::not_null<const gnss::LocationPacket*> data, uint64_t time);

private:
  void write_(gsl::not_null<const gnss::LocationPacket*> data);

  std::ofstream fs_;
};


template<typename T> void Logfile::write(
    gsl::not_null<const gnss::PacketHeader*> header,
    const T* data,
    uint64_t receive_time)
{
  // Negative delay instead of an overflow when receive < transmit
  int64_t transmit_delay = receive_time - header->transmit_time;

  fs_ << header->device_id << ','
      << header->packet_type << ','
      << receive_time << ','
      << transmit_delay << ','
      << header->transmit_system_delay << ','
      << header->transmit_counter << ',';

  write_(data);

  fs_ << '\n';
}


template<typename T> void Logfile::write(
    const T* data,
    uint16_t packet_type,
    uint16_t device_id,
    uint64_t receive_time)
{
  fs_ << device_id << ','
      << packet_type << ','
      << receive_time << ',';

  write_(data);

  fs_ << '\n';
}


}  // namespace logging


#endif  // LOGFILE_H_
