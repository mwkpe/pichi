#ifndef UTIL_H_
#define UTIL_H_


#include <cstdint>
#include <utility>


namespace util {


template<typename T> T take(T& from);

double dm_to_decimal(int degrees, float minutes, char direction);
double dms_to_decimal(int degrees, int minutes, float seconds, char direction);

uint64_t as_utc_unix(int year, int month, int day,
                     int hour, int minute, int second);
double as_utc_unix(int year, int month, int day,
                   int hour, int minute, float second);


}  // namespace util


template<typename T> T util::take(T& from)
{
  T t;
  std::swap(from, t);
  return t;
}


#endif  // UTIL_H_
