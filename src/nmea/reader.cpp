#include "reader.h"


#include <algorithm>
#include <functional>

#include "parser.h"


nmea::Reader::Reader(const Configuration& conf,
                     const Timer& timer,
                     std::condition_variable& data_ready,
                     std::mutex& data_mutex,
                     std::deque<ReadData>& data)
  : conf_{conf},
    timer_{timer},
    nmea_data_ready_{data_ready},
    nmea_data_mutex_{data_mutex},
    nmea_data_{data}
{
}


nmea::Reader::~Reader()
{
}


void nmea::Reader::start()
{
  start_(conf_.gnss_port, conf_.gnss_port_rate);
}


void nmea::Reader::reset()
{
  activity_counter_.store(0);
}


void nmea::Reader::handle_read(gsl::span<char> buffer)
// Reads NMEA sentences from buffer and notifies possible consumer thread
{
  uint64_t systime = timer_.current_systime();
  uint64_t time = timer_.current_time();

  // Prevent crash due to spirit parser's isascii assert when garbage was read
  replace_nonascii(buffer, '0');

  auto sentences = get_sentences(buffer);
  if (!sentences.empty()) {
    // What sentences were received?
    auto typed_sentences = to_typed_sentences(std::move(sentences));
    // Remove sentences disabled by the user
    filter(typed_sentences, conf_);
    activity_counter_.fetch_add(typed_sentences.size());

    // Push data
    std::lock_guard<std::mutex> lk(nmea_data_mutex_);
    for (auto& t : typed_sentences) {
      nmea_data_.emplace_back(time, systime, std::get<0>(t), std::move(std::get<1>(t)));
    }

    // Prevent overflow due to no one consuming the sentences
    while (nmea_data_.size() > 30)
      nmea_data_.pop_front();

    nmea_data_ready_.notify_one();
  }
}


std::vector<std::string> nmea::get_sentences(gsl::span<char> buffer)
// Creates vector of NMEA sentences from buffer
{
  std::vector<std::string> sentences;
  auto pos = std::begin(buffer);
  auto last = std::end(buffer);
  auto sentence_start = pos;
  while (pos != last) {
    pos = std::find_if(pos, last, [](char c) { return c == '\r'; });
    if (pos != last) {
      sentences.push_back({sentence_start, pos});
      std::advance(pos, 1);  // Skip carriage return
      if (pos != last)
        std::advance(pos, 1);  // Skip newline
      sentence_start = pos;
    }
  }
  return sentences;
}


auto nmea::to_typed_sentences(std::vector<std::string>&& sentences)
  -> std::vector<std::tuple<nmea::SentenceType, std::string>>
// Adds type information to sentences
{
  std::vector<std::tuple<nmea::SentenceType, std::string>> typed_sentences;
  for (auto& s : sentences) {
    typed_sentences.emplace_back(nmea::sentence_type(s), std::move(s));
  }
  return typed_sentences;
}


void nmea::filter(std::vector<std::tuple<nmea::SentenceType, std::string>>& sentences,
                  const Configuration& conf)
// Removes sentences from vector that are not enabled in conf
{
  auto new_end = std::remove_if(std::begin(sentences), std::end(sentences),
    [&conf](const std::tuple<nmea::SentenceType, std::string>& t) {
      auto type = std::get<0>(t);
      return (type == nmea::SentenceType::Rmc && !conf.use_msg_rmc) ||
             (type == nmea::SentenceType::Gga && !conf.use_msg_gga) ||
             (type == nmea::SentenceType::Gsv && !conf.use_msg_gsv) ||
             (type == nmea::SentenceType::Unknown && !conf.use_msg_other);
  });
  sentences.erase(new_end, std::end(sentences));
}


void nmea::replace_nonascii(gsl::span<char> s, char c)
{
  // ARM char is unsigned by default
  #ifdef __arm__
    std::replace_if(std::begin(s), std::end(s),
        std::bind(std::greater<char>(), std::placeholders::_1, 127), c);
  #else
    std::replace_if(std::begin(s), std::end(s),
        std::bind(std::less<char>(), std::placeholders::_1, 0), c);
  #endif
}
