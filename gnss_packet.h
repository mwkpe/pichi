#ifndef GNSS_PACKET_H_
#define GNSS_PACKET_H_


#include <cstdint>


namespace gnss {


enum class PacketType : uint16_t {
  Location = 0x10
};


// Mind the packing, these are used for network transmission,
// keep everything 32 and 64-bit aligned and pad manually

struct PacketHeader
{
  uint16_t packet_type;
  uint16_t data_size;
  uint32_t transmit_counter;
  uint64_t transmit_time;
  uint32_t transmit_system_delay;
  uint16_t device_id;
  uint16_t unused_176;
};


struct LocationPacket
{
  double utc_timestamp;
  double latitude;
  double longitude;
};


constexpr uint16_t packet_header_size = sizeof(PacketHeader);
constexpr uint16_t location_data_size = sizeof(LocationPacket);


}  // namespace gnss


#endif  // GNSS_PACKET_H_
