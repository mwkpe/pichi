#include "configuration.h"


#include <string>
#include <iostream>
#include <ios>
#include <sstream>
#include <fstream>
#include <regex>


#include "../ext/json/json.hpp"
using json = nlohmann::json;
using namespace std::string_literals;


pichi::Configuration::Configuration(const std::string& filename)
  : filename_{filename}
{
  std::ifstream fs{filename_};
  json settings;

  if (fs.is_open()) {
    std::stringstream ss;
    ss << fs.rdbuf();
    fs.close();
    settings = json::parse(ss.str());
  }

  auto get = [&settings](const std::string& key, auto def) -> decltype(def) {
    if (settings.count(key)) {
      return settings[key];
    }
    return def;
  };

  gnss_port = get("gnss_port", "/dev/ttyS0"s);
  gnss_port_rate = get("gnss_port_rate", 9600);

  trans_ip = get("trans_ip", "192.168.0.1"s);
  trans_port = get("trans_port", 30001);

  recv_ip = get("recv_ip", "192.168.0.1"s);
  recv_port = get("recv_port", 30001);
  recv_log = get("recv_log", true);
  if (get("recv_log_format", "short"s) == "short")
    recv_log_format = LogFormat::Short;
  else
    recv_log_format = LogFormat::Full;

  log_csv = get("log_csv", true);
  log_gpx = get("log_gpx", true);
  log_csv_force_1hz = get("log_csv_force_1hz", false);
  log_gpx_force_1hz = get("log_gpx_force_1hz", true);
}


void pichi::Configuration::save_to_file() const
{
  json settings;

  settings["gnss_port"] = gnss_port;
  settings["gnss_port_rate"] = gnss_port_rate;

  settings["trans_ip"] = trans_ip;
  settings["trans_port"] = trans_port;

  settings["recv_ip"] = recv_ip;
  settings["recv_port"] = recv_port;
  settings["recv_log"] = recv_log;
  switch (recv_log_format) {
    case LogFormat::Full: settings["recv_log_format"] = "full"s; break;
    case LogFormat::Short: settings["recv_log_format"] = "short"s; break;
  }

  settings["log_csv"] = log_csv;
  settings["log_gpx"] = log_gpx;
  settings["log_csv_force_1hz"] = log_csv_force_1hz;
  settings["log_gpx_force_1hz"] = log_gpx_force_1hz;

  std::ofstream fs{filename_};
  if (fs.is_open()) {
    fs << settings.dump(2);
  }
}
