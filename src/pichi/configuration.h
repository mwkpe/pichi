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
  uint16_t device_id;

  // GNSS tab
  std::string gnss_port;
  uint32_t gnss_port_rate;

  // Transmit tab
  std::string trans_ip;
  uint16_t trans_port;

  // Receive tab
  std::string recv_ip;
  uint16_t recv_port;
  bool recv_log;
  LogFormat recv_log_format;

  // Log tab
  bool log_csv;
  bool log_gpx;
  bool log_csv_force_1hz;
  bool log_gpx_force_1hz;

private:
  std::string filename_{};
};


}  // namespace pichi


#endif  // PICHI_CONFIGURATION_H_
