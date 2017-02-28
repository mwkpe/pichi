#ifndef NMEA_READER_H_
#define NMEA_READER_H_


#include <cstdint>
#include <atomic>
#include <array>
#include <vector>
#include <tuple>
#include <string>
#include <deque>
#include <mutex>
#include <condition_variable>

#include "../ext/gsl.h"

#include "nmea_parser.h"
#include "../base/async_serial_reader.h"
#include "../util/timer.h"


namespace pichi {


inline gsl::span<char> find_sentence(gsl::span<char> buffer)
{
  auto it = std::find_if(std::begin(buffer), std::end(buffer), [](char c) { return c == '\n'; });
  if (it != std::end(buffer))
    return buffer.first(std::distance(std::begin(buffer), it) + 1);  // + 1 to move past \n
  return gsl::span<char>{};
};


struct NmeaSentence
{
  NmeaSentence() = default;
  NmeaSentence(util::TimePoint tp, nmea::SentenceType sentence_type, std::string&& sentence_text)
      : read_time{tp}, type{sentence_type}, text{std::move(sentence_text)} {}
  util::TimePoint read_time;
  nmea::SentenceType type;
  std::string text;
};


class NmeaReader final : public base::AsyncSerialReader
{
public:
  static constexpr size_t BUFFER_SIZE = READ_BUFFER_SIZE * 2;

  NmeaReader(const util::Timer& timer, std::condition_variable& data_ready, std::mutex& data_mutex,
      std::deque<NmeaSentence>& data);
  ~NmeaReader();
  NmeaReader(const NmeaReader&) = delete;
  NmeaReader& operator=(const NmeaReader&) = delete;

  void start(const std::string& port, uint32_t rate);
  uint64_t activity_count() const { return activity_counter_.load(); }

private:
  void reset();
  void reset_buffer();
  void handle_read(gsl::span<char> buffer) override;
  int process_data(gsl::span<char> buffer, util::TimePoint read_time);

  std::array<char, BUFFER_SIZE> buffer_;
  std::array<char, BUFFER_SIZE>::iterator buffer_end_;

  const util::Timer& timer_;
  std::condition_variable& data_ready_;
  std::mutex& data_mutex_;
  std::deque<NmeaSentence>& data_;

  std::atomic<uint64_t> activity_counter_{0};
};


}  // namespace pichi


#endif  // NMEA_READER_H_
