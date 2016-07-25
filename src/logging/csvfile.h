#ifndef CSVFILE_H_
#define CSVFILE_H_


#include <string>
#include <fstream>

#include "../ext/gsl.h"
#include "../gnss/packet.h"

#include "logfile.h"


namespace logging {


class CsvFile final : public LogFile
{
public:
  CsvFile() = default;
  explicit CsvFile(const std::string& filename) : LogFile(filename) {}
  CsvFile(const CsvFile&) = delete;
  CsvFile& operator=(const CsvFile&) = delete;

  bool open(const std::string& filename) { return open_(filename); }

  template<typename T> void write(
      gsl::not_null<const gnss::PacketHeader*> header,
      const T* data,
      uint64_t receive_time);
  template<typename T> void write(
      const T* data,
      uint16_t device_id,
      uint64_t receive_time);
  void write(gsl::not_null<const gnss::LocationPacket*> data, uint64_t time);

private:
  void write_(gsl::not_null<const gnss::LocationPacket*> data);
};


template<typename T> void CsvFile::write(
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


template<typename T> void CsvFile::write(
    const T* data,
    uint16_t device_id,
    uint64_t receive_time)
{
  fs_ << device_id << ','
      << receive_time << ',';

  write_(data);

  fs_ << '\n';
}


}  // namespace logging


#endif  // CSVFILE_H_
