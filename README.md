# Pichi
[![Status](https://img.shields.io/badge/Status-WIP-yellow.svg)](http://www.merriam-webster.com/dictionary/work%20in%20progress)
[![System](https://img.shields.io/badge/System-Raspberry%20Pi-bc1142.svg)](https://www.raspberrypi.org/)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)
[![Standard](https://img.shields.io/badge/C%2B%2B-14-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![I/O](https://img.shields.io/badge/I%2FO-Asio-blue.svg)](http://think-async.com/)
[![GUI](https://img.shields.io/badge/GUI-FLTK-blue.svg)](http://www.fltk.org/)
[![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)](https://opensource.org/licenses/MIT)

A GNSS location transceiver/logger for the Raspberry Pi

Description
---
Pichi is a small program for reading NMEA sentences from a GNSS receiver and logging or transmitting the positonal data, practically turning a Raspberry Pi into a GPS tag.

Requirements
---
A Raspberry Pi or similar device – except for the (optional and easily adaptable/removeable) [system timer](/timer.h) there's no platform-specific code – and a GNSS receiver supporting the NMEA 0183 protocol (RMC or GGA). Developed and tested with a Raspberry Pi 3 (Model B) and a NAVILOCK NL-8002U USB 2.0 Multi GNSS Receiver (u-blox-8).

Currently the program can only read sentences from the device. The receiver itself must be configured with another software (e.g. u-center or similar) to send RMC or GGA sentences in the desired frequency.

Development setup
---
Compilation requires GCC 4.9 (or later with updating the makefile), boost (Spirit), FLTK 1.3 and the non-boost version of Asio. The first two should be part of any recent Raspian.

Install Asio: `sudo apt-get install libasio-dev`<br>
Install FLTK: `sudo apt-get install libfltk1.3-dev`<br>

The Raspberry Pi 2 and 3 should work out of the box. When using a Raspberry Pi 1 a function argument in the [timer.cpp](/timer.cpp#L19) must be changed from `ST_BASE_RPI_2_AND_3` to `ST_BASE_RPI_1` or the system timer will read at the wrong address.

Build the program with `make` (or `make -j6` to save time) and build the nmea parser test with `make test`.

Note: For whatever reason installing FLTK doesn't seem to grab all the required dependencies and the build process may stop with the following error:
```
/usr/bin/ld: cannot find -lXft
/usr/bin/ld: cannot find -lfontconfig
/usr/bin/ld: cannot find -lfontconfig
/usr/bin/ld: cannot find -lXinerama
```
This can be fixed by manually installing `libxft-dev`, `libfontconfig1-dev` and `libxinerama-dev`.

The test of the NMEA parser can be run with `./nmea_parser_test`.

Running the program
---
Start the program with `sudo ./pichi`.<br>
Using sudo is necessary for accessing to the memory location of the the Raspberry Pi's 1MHz [system timer](/timer.cpp#L13). The program works without sudo but all values relying to the system timer (e.g. system delay) will then be zero.

Acknowledgements
---
Pichi is using [FLTK](http://www.fltk.org) for the GUI, [Asio](http://think-async.com/) for network and serial I/O, [doctest](https://github.com/onqtam/doctest) and (some) [cpplint](https://github.com/google/styleguide/tree/gh-pages/cpplint) for testing, and [GSL Lite](https://github.com/martinmoene/gsl-lite).
