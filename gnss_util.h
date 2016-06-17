#ifndef GNSS_UTIL_H_
#define GNSS_UTIL_H_


#include <cstdint>


namespace gnss {
namespace util {


double dm_to_decimal(int degrees, float minutes, char direction);
double dms_to_decimal(int degrees, int minutes, float seconds, char direction);

uint64_t as_utc_unix(int year, int month, int day,
                     int hour, int minute, int second);
double as_utc_unix(int year, int month, int day,
                   int hour, int minute, float second);


}  // namespace util
}  // namespace gnss


#endif  // GNSS_UTIL_H_
