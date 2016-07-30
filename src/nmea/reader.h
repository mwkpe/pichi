#ifndef NMEA_READER_H_
#define NMEA_READER_H_


#include <cstdint>
#include <atomic>
#include <vector>
#include <tuple>
#include <string>
#include <deque>
#include <mutex>
#include <condition_variable>

#include "../ext/gsl.h"

#include "parser.h"
#include "../base/serial_async_reader.h"
#include "../timer.h"


namespace nmea {


struct Sentence
{
  Sentence() = default;
  Sentence(uint64_t time, uint64_t systime, SentenceType type, std::string&& text)
  : time{time}, systime{systime}, type{type}, text{std::move(text)} {}

  uint64_t time;
  uint64_t systime;
  SentenceType type;
  std::string text;
};


class Reader final : public serial::AsyncReader
{
public:
  Reader(const Timer& timer,
         std::condition_variable& data_ready,
         std::mutex& data_mutex,
         std::deque<Sentence>& data);
  ~Reader();
  Reader(const Reader&) = delete;
  Reader& operator=(const Reader&) = delete;

  void start(const std::string& port, uint32_t rate);
  uint64_t activity_count() const { return activity_counter_.load(); }

private:
  void handle_read(gsl::span<char> buffer) override;
  void reset();

  const Timer& timer_;
  std::condition_variable& data_ready_;
  std::mutex& data_mutex_;
  std::deque<Sentence>& data_;

  std::atomic<uint64_t> activity_counter_{0};
};


// Helper

std::vector<std::string> get_sentences(gsl::span<char> buffer);

auto to_typed_sentences(std::vector<std::string>&& sentences)
  -> std::vector<std::tuple<nmea::SentenceType, std::string>>;

void filter(std::vector<std::tuple<nmea::SentenceType, std::string>>& sentences);

void replace_nonascii(gsl::span<char> s, char c);


}  // namespace nmea


#endif  // NMEA_READER_H_
