#ifndef GNSS_PACKET_H
#define GNSS_PACKET_H


#include <cstdint>
#include "nmea_parser.h"


namespace gnss {


enum class PacketType : uint16_t {
  GxRmc = 0x10,
  GxGga,
  GxGsv
};

struct PacketHeader
{
  uint16_t packet_type;
  uint16_t data_length;
  uint32_t transmit_counter;
  uint64_t transmit_time;
  uint32_t transmit_system_delay;
  uint16_t device_id;
};

constexpr std::size_t packet_header_size = sizeof(PacketHeader);
constexpr std::size_t rmc_data_size = sizeof(nmea::RmcData);
constexpr std::size_t gga_data_size = sizeof(nmea::GgaData);
constexpr std::size_t rmc_packet_size = packet_header_size + rmc_data_size;
constexpr std::size_t gga_packet_size = packet_header_size + gga_data_size;


}  // namespace gnss


#endif  // GNSS_PACKET_H
