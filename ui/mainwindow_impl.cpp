// Implementation of functions declared in FLMainWindowD designer
#include "mainwindow.h"


#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#include <FL/Fl.H>

#include "../configuration.h"
#include "../pichi.h"


void MainWindow::apply_settings()
{
  Configuration conf{pichi_->config()};
  try {
    conf.device_id = std::stoul(text_device_id->value());
    conf.gnss_port = text_gnss_port->value();
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
      std::cerr << "Transceiver already running!" << std::endl;
      button_start->value(1);
    }
    else {
      std::string filename;
      if (choice_mode->value() > 0) {
        auto t = std::chrono::high_resolution_clock::now().time_since_epoch();
        filename = std::string("logs/log_") + std::to_string(t.count()) + ".csv";
      }
      switch (choice_mode->value()) {
        case 0: pichi_->start_gnss_transmitter(); break;
        case 1: pichi_->start_gnss_receiver(); break;
        case 2: pichi_->start_gnss_logger(); break;
      }
      button_start->label("Stop");
      last_count_ = 0;
      Fl::add_timeout(1.0, &MainWindow::update_status_callback, this);
    }
  }
  else {
    pichi_->stop();
    button_start->label("Start");
    Fl::remove_timeout(&MainWindow::update_status_callback, this);
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
  else std::cerr << "Can't sync time while transceiver is running!" << std::endl;
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
