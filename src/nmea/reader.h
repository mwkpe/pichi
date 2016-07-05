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
#include "../configuration.h"
#include "../timer.h"


namespace nmea {


struct ReadData
{
  ReadData() = default;
  ReadData(uint64_t read_time, uint64_t read_systime,
           SentenceType sentence_type, std::string&& read_sentence)
  : time{read_time}, systime{read_systime},
    sentence_type{sentence_type}, sentence{std::move(read_sentence)} {}

  uint64_t time;
  uint64_t systime;
  SentenceType sentence_type;
  std::string sentence;
};


class Reader final : public serial::AsyncReader
{
public:
  Reader(const Configuration& conf,
         const Timer& timer,
         std::condition_variable& data_ready,
         std::mutex& data_mutex,
         std::deque<ReadData>& data);
  ~Reader();
  Reader(const Reader&) = delete;
  Reader& operator=(const Reader&) = delete;

  void start();
  uint64_t activity_count() const { return activity_counter_.load(); }

private:
  void handle_read(gsl::span<char> buffer) override;
  void reset();

  const Configuration& conf_;
  const Timer& timer_;
  std::condition_variable& nmea_data_ready_;
  std::mutex& nmea_data_mutex_;
  std::deque<ReadData>& nmea_data_;

  std::atomic<uint64_t> activity_counter_{0};
};


// Helper

std::vector<std::string> get_sentences(gsl::span<char> buffer);

auto to_typed_sentences(std::vector<std::string>&& sentences)
  -> std::vector<std::tuple<nmea::SentenceType, std::string>>;

void filter(std::vector<std::tuple<nmea::SentenceType, std::string>>& sentences,
            const Configuration& conf);

void replace_nonascii(gsl::span<char> s, char c);


}  // namespace nmea


#endif  // NMEA_READER_H_
