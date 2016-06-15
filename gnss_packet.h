#ifndef GNSS_PACKET_H
#define GNSS_PACKET_H


#include <cstdint>
#include "nmea_parser.h"


namespace gnss {


enum class PacketType : uint16_t {
  Location = 0x10
};


struct PacketHeader
{
  uint16_t packet_type;
  uint16_t data_size;
  uint32_t transmit_counter;
  uint64_t transmit_time;
  uint32_t transmit_system_delay;
  uint16_t device_id;
};


struct LocationPacket
{
  uint8_t utc_time_hour;
  uint8_t utc_time_minute;
  float utc_time_second;
  double latitude;
  double longitude;
};


constexpr uint16_t packet_header_size = sizeof(PacketHeader);
constexpr uint16_t location_data_size = sizeof(LocationPacket);


}  // namespace gnss


#endif  // GNSS_PACKET_H
