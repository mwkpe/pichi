#include "configuration.h"


#include <string>
#include <sstream>
#include <fstream>


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

  if (settings.is_null())
    settings = json::object();

  device_id = settings.value("device_id", 1);

  gnss_port = settings.value("gnss_port", "/dev/ttyS0"s);
  gnss_port_rate = settings.value("gnss_port_rate", 9600);

  trans_ip = settings.value("trans_ip", "192.168.0.1"s);
  trans_port = settings.value("trans_port", 30001);

  recv_ip = settings.value("recv_ip", "192.168.0.1"s);
  recv_port = settings.value("recv_port", 30001);
  recv_log = settings.value("recv_log", true);
  if (settings.value("recv_log_format", "short"s) == "short")
    recv_log_format = LogFormat::Short;
  else
    recv_log_format = LogFormat::Full;

  log_csv = settings.value("log_csv", true);
  log_gpx = settings.value("log_gpx", true);
  log_csv_force_1hz = settings.value("log_csv_force_1hz", false);
  log_gpx_force_1hz = settings.value("log_gpx_force_1hz", true);
}


void pichi::Configuration::save_to_file() const
{
  json settings;

  settings["device_id"] = device_id;

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
