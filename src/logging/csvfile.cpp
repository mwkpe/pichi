#include "csvfile.h"


#include <iostream>
#include <iomanip>
#include <ios>


void logging::CsvFile::write(gsl::not_null<const gnss::LocationPacket*> data,
                             uint64_t time)
{
  fs_ << time << ',';
  write_(data);
  fs_ << '\n';
}


void logging::CsvFile::write_(gsl::not_null<const gnss::LocationPacket*> data)
{
  fs_ << std::fixed
      << data->utc_timestamp << ','
      << data->latitude << ','
      << data->longitude
      << std::scientific;  // GCC 4.9 not implemented std::defaultfloat
}
