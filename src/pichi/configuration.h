#ifndef PICHI_CONFIGURATION_H_
#define PICHI_CONFIGURATION_H_


#include <cstdint>
#include <string>


namespace pichi {


enum class LogFormat { Full, Short };


class Configuration
{
public:
  explicit Configuration(const std::string& filename);
  void save_to_file() const;

  // Device tab
  uint16_t device_id{1};

  // GNSS tab
  std::string gnss_port{"/dev/ttyS0"};
  uint32_t gnss_port_rate{9600};

  // Transmit tab
  std::string trans_ip{"192.168.0.1"};
  uint16_t trans_port{30001};

  // Receive tab
  std::string recv_ip{"192.168.0.2"};
  uint16_t recv_port{30001};
  bool recv_log{true};
  LogFormat recv_log_format{LogFormat::Short};

  // Log tab
  bool log_csv{true};
  bool log_gpx{true};
  bool log_csv_force_1hz{false};
  bool log_gpx_force_1hz{true};

private:
  std::string filename_{};
};


}  // namespace pichi


#endif  // PICHI_CONFIGURATION_H_
