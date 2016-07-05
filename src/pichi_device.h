#ifndef PICHI_DEVICE_H_
#define PICHI_DEVICE_H_


#include <cstdint>
#include "gnss/packet.h"


namespace pichi {


class Device
{
public:
  explicit Device(uint16_t id) : id_{id} {}

  uint16_t id() const { return id_; }
  gnss::LocationPacket location() const { return location_; }
  void set_location(const gnss::LocationPacket& location) { location_ = location; }

private:
  uint16_t id_{0};
  gnss::LocationPacket location_;
};


}  // namespace pichi


#endif  // PICHI_DEVICE_H_
