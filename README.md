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
A Raspberry Pi or similar device – except for the (optional and easily adaptable/removeable) [system timer](src/timer.h) there's no platform-specific code – and a GNSS receiver supporting the NMEA 0183 protocol (RMC or GGA). Developed and tested with a Raspberry Pi 3 (Model B) and a NAVILOCK NL-8002U USB 2.0 Multi GNSS Receiver (u-blox-8).

Currently the program can only read sentences from the device. The receiver itself must be configured with another software (e.g. u-center or similar) to send RMC or GGA sentences in the desired frequency.

Running the program
---
Start the program with `sudo ./pichi`.<br>
The program works without sudo but won't be able to read the Raspberry Pi's 1MHz system timer](src/timer.cpp#L14).

Build
---
[Build instructions](/README_BUILD.md)

Acknowledgements
---
Pichi is using [FLTK](http://www.fltk.org) for the GUI, [Asio](http://think-async.com/) for network and serial I/O, [doctest](https://github.com/onqtam/doctest) and [*some*](http://kthx.de/~xeth/pub/non-const.png) [cpplint](https://github.com/google/styleguide/tree/gh-pages/cpplint) for testing, [JSON](https://github.com/nlohmann/json) and [GSL Lite](https://github.com/martinmoene/gsl-lite).
