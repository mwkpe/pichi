#include "timer.h"


#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <chrono>


bool util::Timer::init_sys_time()
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

  st_time = reinterpret_cast<std::uint64_t*>(reinterpret_cast<std::uint8_t*>(m) + TIMER_OFFSET);

  return true;
}


std::uint64_t util::Timer::current_unix_time() const
{
  return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}
