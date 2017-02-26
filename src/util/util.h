#ifndef UTIL_UTIL_H_
#define UTIL_UTIL_H_


#include <cstdint>
#include <utility>

#include "../ext/gsl.h"


namespace util {


double dm_to_decimal(int degrees, float minutes, char direction);
double dms_to_decimal(int degrees, int minutes, float seconds, char direction);

uint64_t as_utc_unix(int year, int month, int day, int hour, int minute, int second);
double as_utc_unix(int year, int month, int day, int hour, int minute, float second);

void replace_nonascii(gsl::span<char> s, char c);

template<typename T> T take(T& from)
{
  T t;
  std::swap(from, t);
  return t;
}


}  // namespace util





#endif  // UTIL_UTIL_H_
