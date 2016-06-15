#ifndef GNSS_UTIL_H
#define GNSS_UTIL_H


#include <cstdint>


namespace gnss {
namespace util {


double dm_to_decimal(uint8_t degrees, float minutes, char direction);
double dms_to_decimal(uint8_t degrees, uint8_t minutes,
                      float seconds, char direction);


}  // namespace util
}  // namespace gnss


#endif  // GNSS_RECEIVER_H
