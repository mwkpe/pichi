#ifndef TIMER_H_
#define TIMER_H_


#include <cstdint>


#define ST_BASE_RPI_1_AND_ZERO 0x20003000
#define ST_BASE_RPI_2_AND_3 0x3F003000
#define TIMER_OFFSET 4


namespace util {


struct TimePoint
{
  // GCC 4.9 doesn't support aggregate initializers
  TimePoint() = default;
  TimePoint(std::uint64_t ut, std::uint64_t st) : unix_time{ut}, sys_time{st} {}
  std::uint64_t unix_time = 0;
  std::uint64_t sys_time = 0;
};


class Timer
{
public:
  Timer() : st_time{&st_dummy} {}

  bool init_sys_time(); // 64-bit microsecond system timer (requires sudo)

  std::uint64_t current_unix_time() const;
  std::uint64_t current_sys_time() const { return *st_time; }
  TimePoint now() const { return TimePoint{current_unix_time(), current_sys_time()}; }

private:
  // 64-bit system timer
  std::uint64_t st_dummy{0};
  std::uint64_t* st_time{nullptr};
};


}  // namespace util


#endif  // TIMER_H_
