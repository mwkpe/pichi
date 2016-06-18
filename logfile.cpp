#include "logfile.h"


#include <string>
#include <iostream>
#include <iomanip>
#include <ios>


logging::Logfile::Logfile(const std::string& filename)
{
  // TODO: Proper file system handling
  fs_.open(std::string("logs/log_") + filename);
  if (!fs_.is_open())
    std::cerr << "Could not open log file, logs directory missing?" << std::endl;
}


void logging::Logfile::write(gsl::not_null<const gnss::LocationPacket*> data,
                             uint64_t receive_time)
{
  fs_ << receive_time << ',';
  write_(data);
  fs_ << '\n';
}


void logging::Logfile::write_(gsl::not_null<const gnss::LocationPacket*> data)
{
  fs_ << std::fixed
      << data->utc_timestamp << ','
      << data->latitude << ','
      << data->longitude
      << std::scientific;  // GCC 4.9 not implemented std::defaultfloat
}
