#ifndef GPXFILE_H_
#define GPXFILE_H_


#include <string>
#include <fstream>

#include "logfile.h"


namespace logging {


class GpxFile final : public LogFile
{
public:
  GpxFile() = default;
  explicit GpxFile(const std::string& filename);
  ~GpxFile();
  GpxFile(const GpxFile&) = delete;
  GpxFile& operator=(const GpxFile&) = delete;

  bool open(const std::string& filename);
  void write_trackpoint(
      double lat, double lon,
      int year, int month, int day,
      int hour, int minute, float second);
  void write_trackpoint(
      double lat, double lon, float ele,
      int year, int month, int day,
      int hour, int minute, float second);

private:
  void write_start_xml();
  void write_end_xml();
  void write_open_track();
  void write_close_track();
};


}  // namespace logging


#endif  // GPXFILE_H_
