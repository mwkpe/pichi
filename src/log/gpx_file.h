#ifndef GPX_FILE_H_
#define GPX_FILE_H_


#include <string>
#include <fstream>

#include "../base/log_file.h"


namespace pichi {


class GpxFile final : public base::LogFile
{
public:
  GpxFile() = default;
  explicit GpxFile(const std::string& filename);
  ~GpxFile();
  GpxFile(const GpxFile&) = delete;
  GpxFile& operator=(const GpxFile&) = delete;

  bool open(const std::string& filename);
  void write_trackpoint( double lat, double lon, int year, int month, int day,
      int hour, int minute, float second);
  void write_trackpoint(double lat, double lon, float ele, int year, int month,
      int day, int hour, int minute, float second);

private:
  void write_start_xml();
  void write_end_xml();
  void write_open_track();
  void write_close_track();
};


}  // namespace pichi


#endif  // GPX_FILE_H_
