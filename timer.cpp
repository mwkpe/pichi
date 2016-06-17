#include "timer.h"


#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <chrono>


bool Timer::systime_init()
{
  auto fd = ::open("/dev/mem", O_RDONLY);
  if (fd == -1) {
    std::cerr << "System timer init error: Can't read /dev/mem, forgot sudo?" << std::endl;
    return false;
  }

  auto* m = ::mmap(nullptr, 4096, PROT_READ, MAP_SHARED, fd, ST_BASE_RPI_2_AND_3);
  if (m == MAP_FAILED) {
    std::cerr << "System timer init error: Can't map memory" << std::endl;
    return false;
  }

  st_time = reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(m) + TIMER_OFFSET);

  return true;
}


uint64_t Timer::current_time() const
{
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}


uint32_t Timer::current_ticks() const
{
  // TODO: Implement access to cpu ticks (LKM?)
  return 1337;
}
