#include <FL/Fl.H>

#include "gnss_transceiver.h"
#include "ui.h"


int main(int argc, char* argv[])
{
  using namespace gnss;
  Transceiver transceiver{Configuration("config.txt")};

  Ui ui(&transceiver);
  ui.show(argc, argv);

  return Fl::run();
}
