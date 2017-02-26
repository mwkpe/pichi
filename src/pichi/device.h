#ifndef PICHI_DEVICE_H_
#define PICHI_DEVICE_H_


/* Represents a real device running the software. */


#include <cstdint>
#include "packet.h"


namespace pichi {


class Device
{
public:
  // A device may have any unique ID except for 0 which always refers to the local device
  static const std::uint16_t LOCAL_DEVICE_ID{0};

  explicit Device(uint16_t id) : id_{id} {}
  uint16_t id() const { return id_; }
  LocationPacket location() const { return location_; }
  void set_location(const LocationPacket& location) { location_ = location; }

private:
  uint16_t id_{0};
  LocationPacket location_;
};


}  // namespace pichi


#endif  // PICHI_DEVICE_H_
