// Implementation of functions declared in FLUID designer

#include "mainwindow.h"


#include <FL/Fl.H>

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#include "../configuration.h"
#include "../gnss_packet.h"
#include "../pichi.h"


void MainWindow::init()
{
  // Must be set or callbacks will be called with nullptr
  choice_display_device->user_data(this);

  display_add_device(Pichi::local_device_id);
  choice_display_device->value(0);
}


void MainWindow::apply_settings()
{
  Configuration conf{pichi_->config()};
  try {
    conf.device_id = std::stoul(text_device_id->value());
    conf.gnss_port = text_gnss_port->value();
    conf.gnss_port_rate = std::stoul(text_gnss_port_rate->value());
    conf.use_msg_rmc = check_rmc->value();
    conf.use_msg_gga = check_gga->value();
    conf.use_msg_gsv = check_gsv->value();
    conf.use_msg_other = check_other->value();
    conf.trans_ip = text_trans_ip->value();
    conf.trans_port = std::stoul(text_trans_port->value());
    conf.recv_ip = text_recv_ip->value();
    conf.recv_port = std::stoul(text_recv_port->value());
    pichi_->set_config(conf);
  }
  catch (const std::invalid_argument& e) {
    std::cerr << "Error applying settings: " << e.what()
              << "\nSettings were not applied." << std::endl;
  }
  catch (const std::out_of_range& e) {
    std::cerr << "Error applying settings: " << e.what()
              << "\nSettings were not applied." << std::endl;
  }
}


void MainWindow::load_settings()
{
  auto& conf = pichi_->config();
  text_device_id->value(std::to_string(conf.device_id).c_str());
  text_gnss_port->value(conf.gnss_port.c_str());
  text_gnss_port_rate->value(std::to_string(conf.gnss_port_rate).c_str());
  check_rmc->value(conf.use_msg_rmc);
  check_gga->value(conf.use_msg_gga);
  check_gsv->value(conf.use_msg_gsv);
  check_other->value(conf.use_msg_other);
  text_trans_ip->value(conf.trans_ip.c_str());
  text_trans_port->value(std::to_string(conf.trans_port).c_str());
  text_recv_ip->value(conf.recv_ip.c_str());
  text_recv_port->value(std::to_string(conf.recv_port).c_str());
}


void MainWindow::button_start_clicked()
{
  if (button_start->value()) {
    // Make sure transceiver isn't running
    if (pichi_->is_active()) {
      std::cerr << "Already running!" << std::endl;
      button_start->value(1);
    }
    else {
      switch (choice_mode->value()) {
        case 0: pichi_->start_gnss_transmitter(); break;
        case 1: pichi_->start_gnss_receiver(); break;
        case 2: pichi_->start_gnss_logger(); break;
        case 3: pichi_->start_gnss_display(); break;
        case 4: pichi_->start_debugger(); break;
      }
      button_start->label("Stop");
      last_count_ = 0;
      Fl::add_timeout(1.0, &MainWindow::update_status_callback, this);

      if (choice_mode->value() < 4) {
        Fl::add_timeout(0.5, &MainWindow::update_display_callback, this);
      }
    }
  }
  else {
    pichi_->stop();
    button_start->label("Start");
    Fl::remove_timeout(&MainWindow::update_status_callback, this);
    Fl::remove_timeout(&MainWindow::update_display_callback, this);
  }
}


void MainWindow::button_apply_clicked()
{
  apply_settings();
}


void MainWindow::button_sync_time_clicked()
{
  // This is just for convenience and not intented as a proper time sync method
  if (!pichi_->is_active()) {
    // Rough time sync via NTP
    std::cout << "Syncing time, please wait..." << std::endl;
    std::thread t{[]{ system("sudo service ntp stop && sudo ntpd -gq && sudo service ntp start"); }};
    t.detach();
  }
  else
    std::cerr << "Can't sync time while running!" << std::endl;
}


void MainWindow::radio_log_clicked()
{
  // TODO: Implement short logging
}


void MainWindow::update_status_callback(void* p)
{
  auto* w = reinterpret_cast<MainWindow*>(p);
  w->update_status();
  Fl::repeat_timeout(1.0, &MainWindow::update_status_callback, w);
}


void MainWindow::update_status()
{
  // Rate (intented to be in hz) only works when MainWindow is updated each second
  auto count = pichi_->activity_count();
  auto rate = count - last_count_;
  last_count_ = count;

  text_rate->value(std::to_string(rate).c_str());
  text_total->value(std::to_string(count).c_str());
}


void MainWindow::update_display_callback(void* p)
{
  auto* w = reinterpret_cast<MainWindow*>(p);
  w->update_display();
  Fl::repeat_timeout(0.5, &MainWindow::update_display_callback, w);
}


void MainWindow::update_display()
{
  bool success = false;
  gnss::LocationPacket location;
  std::tie(success, location) = pichi_->gnss_location(display_device_id_);

  if (success) {
    text_display_utc->value(std::to_string(location.utc_timestamp).c_str());
    text_display_lat->value(std::to_string(location.latitude).c_str());
    text_display_long->value(std::to_string(location.longitude).c_str());
  }

  auto ids = pichi_->new_device_ids();
  for (auto i : ids)
    display_add_device(i);
}


void MainWindow::display_device_changed_callback(Fl_Choice* o, void* p)
{
  std::cout << "rofl: " << o << " : " << p << std::endl;
  auto* w = reinterpret_cast<MainWindow*>(p);
  w->display_device_changed(w->mapped_device_id(o->value()));
}


void MainWindow::display_device_changed(uint16_t id)
{
  display_device_id_ = id;
}


void MainWindow::display_add_device(uint16_t id)
{
  std::string name = std::to_string(id);
  choice_display_device->add(name.c_str(), 0, nullptr);
  int index = choice_display_device->find_index(name.c_str());
  mapped_device_ids_[index] = id;
}


uint16_t MainWindow::mapped_device_id(int index)
{
  return mapped_device_ids_[index];
}
