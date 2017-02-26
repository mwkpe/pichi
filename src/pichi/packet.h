#ifndef PICHI_PACKET_H_
#define PICHI_PACKET_H_


#include <cstdint>


namespace pichi {


enum class PacketType : uint16_t {
  Location = 0x10
};


// Mapped onto buffer for network transmission, keep aligned and pad manually
#pragma pack(4)


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


#pragma pack()


constexpr uint16_t PACKET_HEADER_SIZE = sizeof(PacketHeader);
constexpr uint16_t LOCATION_DATA_SIZE = sizeof(LocationPacket);


}  // namespace pichi


#endif  // PICHI_PACKET_H_
