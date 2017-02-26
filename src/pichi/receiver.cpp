#include "receiver.h"


#include <vector>
#include <algorithm>


pichi::Receiver::Receiver(const util::Timer& timer, std::condition_variable& data_ready,
    std::mutex& data_mutex, std::deque<ReceiveData>& data)
    : timer_{timer}, data_ready_{data_ready}, data_mutex_{data_mutex}, data_{data}
{
}


pichi::Receiver::~Receiver()
{
}


void pichi::Receiver::start(const std::string& ip, std::uint16_t port)
{
  start_(ip, port);
}


void pichi::Receiver::reset()
{
  activity_counter_.store(0);
}


void pichi::Receiver::handle_receive(gsl::span<std::uint8_t> buffer)
{
  ReceiveData rx_data;
  rx_data.systime = timer_.current_systime();
  rx_data.time = timer_.current_time();

  if (buffer.size() > PACKET_HEADER_SIZE) {
    std::memcpy(&rx_data.header, buffer.data(), PACKET_HEADER_SIZE);
    auto ss = buffer.subspan(PACKET_HEADER_SIZE);
    if (ss.size() >= rx_data.header.data_size) {
      std::copy(std::begin(ss), std::end(ss), std::back_inserter(rx_data.data));
      std::lock_guard<std::mutex> lock(data_mutex_);
      data_.push_back(std::move(rx_data));
      activity_counter_.fetch_add(1);

      // Prevent overflow due to no one consuming the data
      while (data_.size() > 30)
        data_.pop_front();

      data_ready_.notify_one();
    }
  }
}
