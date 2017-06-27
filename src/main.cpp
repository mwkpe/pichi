#include <FL/Fl.H>
#include "ext/cxxopts.hpp"

#include "pichi/configuration.h"
#include "pichi/pichi.h"
#include "ui/mainwindow.h"


int main(int argc, char* argv[])
{
  pichi::Pichi pichi{pichi::Configuration{"config.json"}};
  bool nogui = false;
  pichi::StartMode start_mode = pichi::StartMode::Transmit;

  try {
    cxxopts::Options options("Pichi", "GNSS location logger/transceiver");
    options.add_options()
      ("nogui", "Run program without GUI", cxxopts::value<bool>(nogui))
      ("transmit", "Transmit positional data read from serial port")
      ("receive", "Receive and log positional data")
      ("log", "Log positional data read from serial port")
      ("debug", "Display data read from serial port")
    ;

    options.parse(argc, argv);

    if (nogui) {
      if (options.count("transmit") + options.count("receive") +
          options.count("log") + options.count("debug") != 1)
        throw pichi::Error{"Error parsing command line options:\n"
            "Use --nogui and either --transmit, --receive, --log or --debug"};

      if (options.count("transmit")) start_mode = pichi::StartMode::Transmit;
      else if (options.count("receive")) start_mode = pichi::StartMode::Receive;
      else if (options.count("log")) start_mode = pichi::StartMode::Log;
      else if (options.count("debug")) start_mode = pichi::StartMode::Debug;
    }
  }
  catch (const cxxopts::OptionException& e) {
    std::cout << "Error parsing command line options:\n" << e.what() << std::endl;
    return 1;
  }
  catch (const pichi::Error& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  if (nogui) {
    try {
      switch (start_mode) {
        case pichi::StartMode::Transmit: pichi.start_transmitter(); break;
        case pichi::StartMode::Receive: pichi.start_receiver(); break;
        case pichi::StartMode::Log: pichi.start_logger(); break;
        case pichi::StartMode::Debug: pichi.start_debug_mode(); break;
        default: throw pichi::Error{"Unknown start mode"};
      }

      std::cout << "Running! Enter anything to stop the program!" << std::endl;
      std::cin.ignore();  // Wait in main thread
    }
    catch (const pichi::Error& e) {
      std::cerr << e.what() << std::endl;
      return 1;
    }
  }
  else {
    MainWindow window{&pichi};
    window.show(argc, argv);
    return Fl::run();
  }

  return 0;
}
