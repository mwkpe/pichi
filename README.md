# Pichi
[![Language](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)
[![Standard](https://img.shields.io/badge/C%2B%2B-14-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![I/O](https://img.shields.io/badge/I%2FO-Asio-blue.svg)](http://think-async.com/)
[![GUI](https://img.shields.io/badge/GUI-FLTK-blue.svg)](http://www.fltk.org/)
[![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)](https://opensource.org/licenses/MIT)
[![System](https://img.shields.io/badge/System-Raspberry%20Pi-bc1142.svg)](https://www.raspberrypi.org/)

A GNSS location logger/transceiver for the Raspberry Pi

Description
---
Pichi is a program for reading and parsing NMEA sentences from a GNSS receiver and logging (CSV, GPX) or transmitting (UDP) the positonal data.

Requirements
---
A Raspberry Pi or similar device and a GNSS receiver supporting the NMEA 0183 protocol (RMC sentences).

Build
---
[Build instructions](/README_BUILD.md)

Usage
---
[Usage instructions](/README_USAGE.md)

Acknowledgements
---
Pichi is using the [FLTK](http://www.fltk.org) GUI toolkit, [Asio](http://think-async.com/) for network and serial I/O, [Spirit](http://boost-spirit.com) for text parsing, [cxxopts](https://github.com/jarro2783/cxxopts) for parsing command line options, [doctest](https://github.com/onqtam/doctest) and [cpplint](https://github.com/google/styleguide/tree/gh-pages/cpplint) for testing, and also [fmt](https://github.com/fmtlib/fmt), [JSON](https://github.com/nlohmann/json) and [GSL Lite](https://github.com/martinmoene/gsl-lite).
