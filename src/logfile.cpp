#include "logfile.h"


#include <string>
#include <iostream>
#include <iomanip>
#include <ios>


logging::Logfile::Logfile(const std::string& filename)
{
  open(filename);
}


logging::Logfile::~Logfile()
{
  // Stream will close itself
  if (fs_.is_open())
    std::cout << "Logfile closed" << std::endl;
}


bool logging::Logfile::open(const std::string& filename)
{
  // TODO: Proper file system handling
  fs_.open(std::string("logs/log_") + filename);
  if (!is_open())
    std::cerr << "Could not open log file, logs directory missing?" << std::endl;
  else
    std::cout << "Logfile opened" << std::endl;

  return is_open();
}


void logging::Logfile::write(gsl::not_null<const gnss::LocationPacket*> data,
                             uint64_t time)
{
  fs_ << time << ',';
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
