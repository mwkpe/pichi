#include <FL/Fl.H>

#include "pipoint.h"
#include "ui.h"


int main(int argc, char* argv[])
{
  PiPoint pipoint{Configuration("config.txt")};

  Ui ui(&pipoint);
  ui.show(argc, argv);

  return Fl::run();
}
