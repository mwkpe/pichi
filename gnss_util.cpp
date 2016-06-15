#include "gnss_util.h"


double gnss::util::dm_to_decimal(uint8_t degrees, float minutes, char direction)
{
  auto decimal = static_cast<double>(degrees) + minutes / 60.0;
  if (direction == 'S' || direction == 'W')
    decimal *= -1.0;
  return decimal;
}


double gnss::util::dms_to_decimal(uint8_t degrees, uint8_t minutes,
                                  float seconds, char direction)
{
  return dm_to_decimal(degrees, minutes + seconds / 3600.0, direction);
}
