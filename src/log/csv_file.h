#ifndef CSV_FILE_H_
#define CSV_FILE_H_


#include <cstdint>
#include <string>
#include <fstream>

#include "../ext/gsl.h"

#include "../base/log_file.h"
#include "../pichi/packet.h"


namespace pichi {


class CsvFile final : public base::LogFile
{
public:
  CsvFile() = default;
  explicit CsvFile(const std::string& filename) : LogFile(filename) {}
  CsvFile(const CsvFile&) = delete;
  CsvFile& operator=(const CsvFile&) = delete;

  bool open(const std::string& filename) { return open_(filename); }

  template<typename T> void write(gsl::not_null<const PacketHeader*> header, const T* data,
      std::uint64_t receive_time);
  template<typename T> void write(const T* data, std::uint16_t device_id,
      std::uint64_t receive_time);
  void write(gsl::not_null<const LocationPacket*> data, std::uint64_t time);

private:
  void write_(gsl::not_null<const LocationPacket*> data);
};


template<typename T> void CsvFile::write(gsl::not_null<const PacketHeader*> header,
    const T* data, std::uint64_t receive_time)
{
  // Negative delay instead of an overflow when receive < transmit
  std::int64_t transmit_delay = receive_time - header->transmit_time;

  fs_ << header->device_id << ','
      << header->packet_type << ','
      << receive_time << ','
      << transmit_delay << ','
      << header->transmit_system_delay << ','
      << header->transmit_counter << ',';

  write_(data);

  fs_ << '\n';
}


template<typename T> void CsvFile::write(const T* data, std::uint16_t device_id,
    std::uint64_t receive_time)
{
  fs_ << device_id << ','
      << receive_time << ',';

  write_(data);

  fs_ << '\n';
}


}  // namespace logging


#endif  // CSV_FILE_H_
