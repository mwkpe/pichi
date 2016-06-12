#include "configuration.h"


#include <iostream>
#include <ios>
#include <sstream>
#include <fstream>
#include <regex>


Configuration::Configuration(const std::string& filename) : filename_(filename)
{
  std::ifstream fs{filename};
  std::string cfg;

  if (fs.is_open()) {
    std::stringstream ss;
    ss << fs.rdbuf();
    cfg = ss.str();
    fs.close();
  }

  if (!cfg.empty()) {
    auto read_cfg = [&cfg](const std::regex& rx) -> std::string {
      std::smatch match;
      std::regex_search(cfg, match, rx);
      if (match.size() == 2)
        return match[1].str();
      return std::string();
    };

    try {
      device_id = std::stoul(read_cfg(std::regex{R"(device_id=([0-9]+))"}));
      gnss_port = read_cfg(std::regex{R"(gnss_port=([a-zA-Z0-9-_\/]+))"});
      gnss_port_rate = std::stoul(read_cfg(std::regex{R"(gnss_port_rate=([0-9]+))"}));
      use_msg_rmc = read_cfg(std::regex{R"(use_msg_rmc=(true|false))"}) == "true";
      use_msg_gga = read_cfg(std::regex{R"(use_msg_gga=(true|false))"}) == "true";
      use_msg_gsv = read_cfg(std::regex{R"(use_msg_gsv=(true|false))"}) == "true";
      use_msg_other = read_cfg(std::regex{R"(use_msg_other=(true|false))"}) == "true";
      trans_ip = read_cfg(std::regex{R"(trans_ip=([0-9a-fA-F\.\:]+))"});
      trans_port = std::stoul(read_cfg(std::regex{R"(trans_port=([0-9]+))"}));
      recv_ip = read_cfg(std::regex{R"(recv_ip=([0-9a-fA-F\.\:]+))"});
      recv_port = std::stoul(read_cfg(std::regex{R"(recv_port=([0-9]+))"}));
      log_recv = read_cfg(std::regex{R"(log_recv=(true|false))"}) == "true";
    }
    catch (const std::invalid_argument& e) {
      std::cerr << "Error loading config: " << e.what()
                << "\nUsing default values." << std::endl;
      *this = Configuration{};
    }
    catch (const std::out_of_range& e) {
      std::cerr << "Error loading config: " << e.what()
                << "\nUsing default values." << std::endl;
      *this = Configuration{};
    }
  }
}


void Configuration::save_to_file() const
{
  std::ofstream fs{filename_};
  if (fs.is_open()) {
    fs << std::boolalpha
       <<"# Device\n"
       << "device_id=" << device_id << '\n'
       << "\n# GNSS\n"
       << "gnss_port=" << gnss_port << '\n'
       << "gnss_port_rate=" << gnss_port_rate << '\n'
       << "use_msg_rmc=" << use_msg_rmc << '\n'
       << "use_msg_gga=" << use_msg_gga << '\n'
       << "use_msg_gsv=" << use_msg_gsv << '\n'
       << "use_msg_other=" << use_msg_other << '\n'
       << "\n# Transmitter\n"
       << "trans_ip=" << trans_ip << '\n'
       << "trans_port=" << trans_port << '\n'
       << "\n# Receiver\n"
       << "recv_ip=" << recv_ip << '\n'
       << "recv_port=" << recv_port << '\n'
       << "\n# Logging\n"
       << "log_recv=" << log_recv;
  }
}
