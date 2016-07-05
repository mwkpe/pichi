#include "util.h"


#include <ctime>
#include <cmath>


double util::dm_to_decimal(int degrees, float minutes, char direction)
{
  auto decimal = static_cast<double>(degrees) + minutes / 60.0;
  if (direction == 'S' || direction == 'W')
    decimal *= -1;
  return decimal;
}


double util::dms_to_decimal(int degrees, int minutes,
                            float seconds, char direction)
{
  return dm_to_decimal(degrees, minutes + seconds / 3600.0, direction);
}


uint64_t util::as_utc_unix(int year, int month, int day,
                           int hour, int minute, int second)
{
  tm time;
  time.tm_year = year - 1900;  // Years since 1900
  time.tm_mon = month - 1;  // Months since January (0-11)
  time.tm_mday = day;
  time.tm_hour = hour;
  time.tm_min = minute;
  time.tm_sec = second;

  return static_cast<uint64_t>(timegm(&time));
}


double util::as_utc_unix(int year, int month, int day,
                         int hour, int minute, float second)
{
  float integral_part;
  float fractional_part = std::modf(second, &integral_part);
  auto utc_second = as_utc_unix(year, month, day, hour, minute,
                                static_cast<int>(integral_part));
  return static_cast<double>(utc_second) + fractional_part;
}
