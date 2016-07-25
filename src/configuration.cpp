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

  trans_ip = get("trans_ip", std::string("192.168.0.1"));
  trans_port = get("trans_port", 30001);

  recv_ip = get("recv_ip", std::string("192.168.0.1"));
  recv_port = get("recv_port", 30001);
  recv_log = get("recv_log", true);
  recv_log_format = get("recv_log_format", std::string("short"));

  log_csv = get("log_csv", true);
  log_gpx = get("log_gpx", true);
  log_csv_force_1hz = get("log_csv_force_1hz", false);
  log_gpx_force_1hz = get("log_gpx_force_1hz", true);
}


void Configuration::save_to_file() const
{
  json settings;

  settings["gnss_port"] = gnss_port;
  settings["gnss_port_rate"] = gnss_port_rate;

  settings["trans_ip"] = trans_ip;
  settings["trans_port"] = trans_port;

  settings["recv_ip"] = recv_ip;
  settings["recv_port"] = recv_port;
  settings["recv_log"] = recv_log;
  settings["recv_log_format"] = recv_log_format;

  settings["log_csv"] = log_csv;
  settings["log_gpx"] = log_gpx;
  settings["log_csv_force_1hz"] = log_csv_force_1hz;
  settings["log_gpx_force_1hz"] = log_gpx_force_1hz;

  std::ofstream fs{filename_};
  if (fs.is_open()) {
    fs << settings.dump(2);
  }
}
