#ifndef TIMER_H_
#define TIMER_H_


#include <cstdint>


#define ST_BASE_RPI_1_AND_ZERO 0x20003000
#define ST_BASE_RPI_2_AND_3 0x3F003000
#define TIMER_OFFSET 4


class Timer
{
public:
  Timer() : st_time(&st_dummy) {}

  // 64-bit system timer (requires sudo)
  bool systime_init();

  uint64_t current_time() const;
  uint64_t current_systime() const { return *st_time; }
  uint32_t current_ticks() const;

private:
  // 64-bit system timer
  uint64_t st_dummy{0};
  uint64_t* st_time{nullptr};
};


#endif  // TIMER_H_
