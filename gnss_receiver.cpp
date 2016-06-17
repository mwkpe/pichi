#include "gnss_receiver.h"


#include <vector>
#include <algorithm>


gnss::Receiver::Receiver(const Configuration& conf,
                         const Timer& timer,
                         std::condition_variable& data_ready,
                         std::mutex& data_mutex,
                         std::deque<ReceiveData>& data)
  : conf_{conf},
    timer_{timer},
    gnss_data_ready_{data_ready},
    gnss_data_mutex_{data_mutex},
    gnss_data_{data}
{
}


gnss::Receiver::~Receiver()
{
}


void gnss::Receiver::start()
{
  start_(conf_.recv_ip, conf_.recv_port);
}


void gnss::Receiver::reset()
{
  activity_counter_.store(0);
}


void gnss::Receiver::handle_receive(gsl::span<uint8_t> buffer)
{
  ReceiveData received;
  received.systime = timer_.current_systime();
  received.time = timer_.current_time();

  if (buffer.size() > packet_header_size) {
    std::memcpy(&received.header, buffer.data(), packet_header_size);
    auto ss = buffer.subspan(packet_header_size);
    std::copy(std::begin(ss), std::end(ss), std::back_inserter(received.data));
    gnss_data_.push_back(std::move(received));
    activity_counter_.fetch_add(1);

    // Prevent overflow due to no one consuming the data
    while (gnss_data_.size() > 30)
      gnss_data_.pop_front();

    gnss_data_ready_.notify_one();
  }
}