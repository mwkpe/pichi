#include "configuration.h"


#include <iostream>
#include <ios>
#include <sstream>
#include <fstream>
#include <regex>


#include "ext/json/json.hpp"
using json = nlohmann::json;


Configuration::Configuration(const std::string& filename)
  : filename_{filename}
{
  std::ifstream fs{filename_};
  std::string cfg;
  json settings;

  if (fs.is_open()) {
    std::stringstream ss;
    ss << fs.rdbuf();
    cfg = ss.str();
    fs.close();
    settings = json::parse(cfg);
  }

  auto get = [&settings](const std::string& key, auto def)
    -> decltype(def)
  {
    if (settings.count(key)) {
      return settings[key];
    }
    return def;
  };

  gnss_port = get("gnss_port", std::string("/dev/ttyS0"));
  gnss_port_rate = get("gnss_port_rate", 9600);

  use_msg_rmc = get("use_msg_rmc", true);
  use_msg_gga = get("use_msg_gga", false);
  use_msg_gsv = get("use_msg_gsv", false);
  use_msg_other = get("use_msg_other", false);

  trans_ip = get("trans_ip", std::string("192.168.0.1"));
  trans_port = get("trans_port", 30001);

  recv_ip = get("recv_ip", std::string("192.168.0.1"));
  recv_port = get("recv_port", 30001);

  log_recv = get("log_recv", true);
  log_format = get("log_format", std::string("short"));
}


void Configuration::save_to_file() const
{
  json settings;

  settings["gnss_port"] = gnss_port;
  settings["gnss_port_rate"] = gnss_port_rate;

  settings["use_msg_rmc"] = use_msg_rmc;
  settings["use_msg_gga"] = use_msg_gga;
  settings["use_msg_gsv"] = use_msg_gsv;
  settings["use_msg_other"] = use_msg_other;

  settings["trans_ip"] = trans_ip;
  settings["trans_port"] = trans_port;

  settings["recv_ip"] = recv_ip;
  settings["recv_port"] = recv_port;

  settings["log_recv"] = log_recv;
  settings["log_format"] = log_format;

  std::ofstream fs{filename_};
  if (fs.is_open()) {
    fs << settings.dump(2);
  }
}
