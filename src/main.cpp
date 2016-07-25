#include <FL/Fl.H>
#include "ext/cxxopts.hpp"

#include "pichi.h"
#include "ui/mainwindow.h"


int main(int argc, char* argv[])
{
  Pichi pichi{Configuration{"config.json"}};
  bool nogui = false;

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
      int total = options.count("transmit") + options.count("receive") +
                  options.count("log") + options.count("debug");
      if (total == 1) {
        if (options.count("transmit")) pichi.start_gnss_transmitter();
        if (options.count("receive")) pichi.start_gnss_receiver();
        if (options.count("log")) pichi.start_location_logger();
        if (options.count("debug")) pichi.start_debugger();

        std::cout << "Running! Enter anything to stop the program!"
                  << std::endl;
        int anything;
        std::cin >> anything;
      }
      else {
        std::cout << "Error parsing command line options:\n"
                  << "Use --nogui and either --transmit, --receive, --log or --debug"
                  << std::endl;
        return 0;
      }
    }
  }
  catch (const cxxopts::OptionException& e) {
    std::cout << "Error parsing command line options:\n" << e.what() << std::endl;
    return 0;
  }

  if (!nogui) {
    MainWindow window{&pichi};
    window.show(argc, argv);
    return Fl::run();
  }

  return 0;
}
