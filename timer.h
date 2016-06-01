#ifndef TIMER_H
#define TIMER_H


#include <cstdint>


#define ST_BASE_RPI_1 0x20003000
#define ST_BASE_RPI_2 0x3F003000
#define ST_BASE_RPI_3 0x3F003000
#define TIMER_OFFSET 4


class Timer
{
public:
  Timer() : st_time(&st_dummy) {}

  // 64-bit system timer (requires sudo)
  bool st_init();
  uint64_t st_now() const { return *st_time; }

private:
  // 64-bit system timer
  uint64_t st_dummy{0};
  uint64_t* st_time{nullptr};
};


#endif  // TIMER_H
