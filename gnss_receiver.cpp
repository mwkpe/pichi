#include "gnss_receiver.h"


#include <vector>


gnss::Receiver::Receiver(const Configuration& conf,
                         const Timer& timer,
                         std::condition_variable& data_ready,
                         std::mutex& data_mutex,
                         std::deque<GnssData>& data)
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
  uint64_t systime = timer_.current_systime();
  uint64_t time = timer_.current_time();
  std::vector<uint8_t> data{std::begin(buffer), std::end(buffer)};
  if (data.size() >= packet_header_size) {
    const auto* header = reinterpret_cast<const PacketHeader*>(data.data());
    if (header->data_length <= data.size() - packet_header_size) {
      gnss_data_.emplace_back(time, systime, static_cast<PacketType>(header->packet_type), std::move(data));
      activity_counter_.fetch_add(1);

      // Prevent overflow due to no one consuming the data
      while (gnss_data_.size() > 30)
        gnss_data_.pop_front();

      gnss_data_ready_.notify_one();
    }
  }
}
