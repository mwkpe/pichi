#ifndef PICHI_PACKET_H_
#define PICHI_PACKET_H_


#include <cstdint>


namespace pichi {


enum class PacketType : uint16_t {
  Location = 0x10
};


// Mapped onto buffer for network transmission
struct alignas(8) PacketHeader
{
  uint16_t device_id;
  uint16_t packet_type;
  uint16_t data_size;
  uint32_t transmit_counter;
  uint32_t transmit_system_delay;
  uint64_t transmit_time;
};


struct alignas(8) LocationPacket
{
  double utc_timestamp;
  double latitude;
  double longitude;
};


constexpr uint16_t PACKET_HEADER_SIZE = sizeof(PacketHeader);
constexpr uint16_t LOCATION_DATA_SIZE = sizeof(LocationPacket);


}  // namespace pichi


#endif  // PICHI_PACKET_H_
