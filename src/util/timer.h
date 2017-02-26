#ifndef TIMER_H_
#define TIMER_H_


#include <cstdint>


#define ST_BASE_RPI_1_AND_ZERO 0x20003000
#define ST_BASE_RPI_2_AND_3 0x3F003000
#define TIMER_OFFSET 4


namespace util {


class Timer
{
public:
  Timer() : st_time{&st_dummy} {}

  // 64-bit microsecond system timer (requires sudo)
  bool systime_init();

  std::uint64_t current_time() const;
  std::uint64_t current_systime() const { return *st_time; }

private:
  // 64-bit system timer
  std::uint64_t st_dummy{0};
  std::uint64_t* st_time{nullptr};
};


}  // namespace util


#endif  // TIMER_H_
