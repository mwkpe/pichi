#include "logfile.h"


Logfile::Logfile(const std::string& filename)
{
  fs_.open(filename);
}


void Logfile::write(gsl::not_null<const gnss::LocationPacket*> data)
{
  fs_ << static_cast<int>(data->utc_time_hour) << ','
      << static_cast<int>(data->utc_time_minute) << ','
      << data->utc_time_second << ','
      << data->latitude << ','
      << data->longitude;
}
