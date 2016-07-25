# Pichi
[![Status](https://img.shields.io/badge/Status-WIP-yellow.svg)](http://www.merriam-webster.com/dictionary/work%20in%20progress)
[![System](https://img.shields.io/badge/System-Raspberry%20Pi-bc1142.svg)](https://www.raspberrypi.org/)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)](https://isocpp.org/)
[![Standard](https://img.shields.io/badge/C%2B%2B-14-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![I/O](https://img.shields.io/badge/I%2FO-Asio-blue.svg)](http://think-async.com/)
[![GUI](https://img.shields.io/badge/GUI-FLTK-blue.svg)](http://www.fltk.org/)
[![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)](https://opensource.org/licenses/MIT)

A GNSS location logger/transceiver for the Raspberry Pi

Description
---
Pichi is a small program for reading and parsing NMEA sentences from a GNSS receiver and logging (CSV, GPX) or transmitting (UDP) the positonal data, turning a Raspberry Pi into a GPS tag.

Status
---
Basically does what it says on the tin ... but it's just a personal RPi project and may or may not work, kill your cat, or rend the fabric of the space time continuum.

Requirements
---
A Raspberry Pi or similar device – except for the optional and easily adaptable/removeable [system timer](src/timer.h) there's no platform-specific code – and a GNSS receiver supporting the NMEA 0183 protocol (RMC sentences). Developed and tested with a Raspberry Pi 3 (Model B) and Zero, and a NAVILOCK NL-8002U USB 2.0 Multi GNSS Receiver (u-blox-8).

Currently the program can only read sentences from the device. The receiver itself must be configured with another software (e.g. u-center or similar) to send RMC sentences in the desired frequency. But there's an option to force 1Hz when logging.

Build
---
[Build instructions](/README_BUILD.md)

Usage
---
[Usage instructions](/README_USAGE.md)

Acknowledgements
---
Pichi is using the [FLTK](http://www.fltk.org) GUI toolkit, [Asio](http://think-async.com/) for network and serial I/O, [Spirit](http://boost-spirit.com) for text parsing, [cxxopts](https://github.com/jarro2783/cxxopts) for parsing command line options, [doctest](https://github.com/onqtam/doctest) and [cpplint](https://github.com/google/styleguide/tree/gh-pages/cpplint) for testing, and [fmt](https://github.com/fmtlib/fmt), [JSON](https://github.com/nlohmann/json) and [GSL Lite](https://github.com/martinmoene/gsl-lite).
