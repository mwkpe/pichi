#ifndef CONFIGURATION_H
#define CONFIGURATION_H


#include <cstdint>
#include <string>


class Configuration
{
public:
  Configuration() = default;
  explicit Configuration(const std::string& filename);
  void save_to_file() const;

  uint16_t device_id{1};

  std::string gnss_port{"/dev/ttyS0"};
  uint32_t gnss_port_rate = 115200;
  bool use_msg_rmc{true};
  bool use_msg_gga{false};
  bool use_msg_gsv{false};
  bool use_msg_other{false};

  std::string trans_ip{"192.168.0.1"};
  uint16_t trans_port{30001};

  std::string recv_ip{"192.168.0.2"};
  uint16_t recv_port{30001};

  bool log_recv{true};

private:
  std::string filename_{};
};


#endif  // CONFIGURATION_H
