#include "csv_file.h"


#include <iostream>
#include <iomanip>
#include <ios>


void pichi::CsvFile::write(gsl::not_null<const LocationPacket*> data,
    std::uint64_t time)
{
  fs_ << time << ',';
  write_(data);
  fs_ << '\n';
}


void pichi::CsvFile::write_(gsl::not_null<const LocationPacket*> data)
{
  fs_ << std::fixed
      << data->utc_timestamp << ','
      << data->latitude << ','
      << data->longitude
      << std::scientific;  // GCC 4.9 not implemented std::defaultfloat
}
