#include "gpx_file.h"


#include "../ext/fmt/format.h"


pichi::GpxFile::GpxFile(const std::string& filename)
  : LogFile(filename)
{
  write_start_xml();
  write_open_track();
}


pichi::GpxFile::~GpxFile()
{
  write_close_track();
  write_end_xml();
}


bool pichi::GpxFile::open(const std::string& filename)
{
  if (open_(filename))
  {
    write_start_xml();
    write_open_track();
    return true;
  }

  return false;
}


void pichi::GpxFile::write_trackpoint(double lat, double lon, float ele,
    int year, int month, int day, int hour, int minute, float second)
{
  fs_ << fmt::format(
      R"(<trkpt lat="{:f}" lon="{:f}">)"
      R"(<ele>{:3.1f}</ele>)"
      R"(<time>{:4}-{:02}-{:02}T{:02}:{:02}:{:06.3f}Z</time>)"
      R"(</trkpt>)" "\n",
      lat, lon, ele, year, month, day, hour, minute, second);
}


void pichi::GpxFile::write_trackpoint(double lat, double lon,
    int year, int month, int day, int hour, int minute, float second)
{
  fs_ << fmt::format(
      R"(<trkpt lat="{:f}" lon="{:f}">)"
      R"(<time>{:4}-{:02}-{:02}T{:02}:{:02}:{:06.3f}Z</time>)"
      R"(</trkpt>)" "\n",
      lat, lon, year, month, day, hour, minute, second);
}


void pichi::GpxFile::write_start_xml()
{
  fs_ <<
      R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>)" "\n"
      R"(<gpx xmlns="http://www.topografix.com/GPX/1/1" )"
      R"(version="1.1" )"
      R"(creator="Pichi" )"
      R"(xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" )"
      R"(xsi:schemaLocation="http://www.topografix.com/GPX/1/1/gpx.xsd">)" "\n";
}


void pichi::GpxFile::write_end_xml()
{
  fs_ << "</gpx>\n";
}


void pichi::GpxFile::write_open_track()
{
  fs_ << "<trk>\n<trkseg>\n";
}


void pichi::GpxFile::write_close_track()
{
  fs_ << "</trkseg>\n</trk>\n";
}
