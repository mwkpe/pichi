#include "timer.h"


#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>


bool Timer::st_init()
{
  auto fd = ::open("/dev/mem", O_RDONLY);
  if (fd == -1) {
    std::cerr << "System timer init error: Can't read /dev/mem. Forgot sudo?" << std::endl;
    return false;
  }

  auto* m = ::mmap(nullptr, 4096, PROT_READ, MAP_SHARED, fd, ST_BASE_RPI_3);
  if (m == MAP_FAILED) {
    std::cerr << "System timer init error: Can't map memory." << std::endl;
    return false;
  }

  st_time = reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(m) + TIMER_OFFSET);

  return true;
}
