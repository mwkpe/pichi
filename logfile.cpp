#include "logfile.h"


#include <string>
#include <regex>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <ios>


logging::Logfile::Logfile(const std::string& filename)
{
  fs_.open(filename);
}


void logging::Logfile::write(gsl::not_null<const gnss::LocationPacket*> data)
{
  write_(data);
  fs_ << '\n';
}


void logging::Logfile::write_(gsl::not_null<const gnss::LocationPacket*> data)
{
  fs_ << std::fixed
      << std::setprecision(3)
      << data->utc_timestamp
      << std::setprecision(6)
      << ','
      << data->latitude << ','
      << data->longitude
      << std::scientific;
}
