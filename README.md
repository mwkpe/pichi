# PiPoint
A GNSS location transceiver for the Raspberry Pi

A small program for reading NMEA sentences from a GNSS receiver. The data can be logged, or transmitted and received via UDP, basically turning the Rasperry Pi into a GPS tag.

Requirements
---
A Raspberry Pi or similar device – except for the [system timer](/timer.h) there's no platform-specific code – and a GNSS receiver supporting the NMEA 0183 protocol (RMC or GGA). Developed and tested with a Raspberry Pi 3 (Model B) and a NAVILOCK NL-8002U USB 2.0 GNSS Receiver (u-blox-8).

Currently the program can only read sentences from the device. The receiver itself must be configured with another software (e.g. u-center or similar) to send RMC or GGA sentences.

Development setup
---
Compilation requires GCC 4.9 (or later with updating the makefile), boost, FLTK 1.3 and the non-boost version of Asio. The first two should be part of any recent Raspian.

Install Asio: `sudo apt-get install libasio-dev`<br>
Install FLTK: `sudo apt-get install libfltk1.3-dev`<br>

The Raspberry Pi 2 and 3 should work out of the box. When using a Raspberry Pi 1 you must change a function argument in the [timer.cpp](/timer.cpp#L18) from `ST_BASE_RPI_2_AND_3` to `ST_BASE_RPI_1` or the system timer will read at the wrong address.

Build the program with `make` and build the nmea parser test with `make test`.

Note: For whatever reason installing FLTK doesn't seem to grab all the required dependencies and the build process may stop with the following error:
```
/usr/bin/ld: cannot find -lXft
/usr/bin/ld: cannot find -lfontconfig
/usr/bin/ld: cannot find -lfontconfig
/usr/bin/ld: cannot find -lXinerama
```
This can be fixed by manually installing `libxft-dev`, `libfontconfig1-dev` and `libxinerama-dev`.

Running the program
---
Start the program with `sudo ./pipoint`. Using sudo is necessary for accessing to the memory location of the the Raspberry Pi's 1MHz [system timer](/timer.cpp). The program works without sudo but all values relying to the system timer (e.g. system delay) will then be zero.

The test of the NMEA parser can be run with `./nmea_parser_test`.

Acknowledgements
---
PiPoint is using [FLTK](http://www.fltk.org) for the GUI, [doctest](https://github.com/onqtam/doctest) for testing, and [GSL Lite](https://github.com/martinmoene/gsl-lite).
