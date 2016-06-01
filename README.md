# PiPoint
A GNSS location transceiver for the Raspberry Pi

A small program for reading and parsing NMEA sentences from a GNSS receiver and transmitting the data via UDP.

Development setup
---
Compilation requires GCC 4.9 (or later with updating the makefile), boost, FLTK 1.3 and the non-boost version of Asio. The first two should be part of any recent Raspian.

Install Asio: `sudo apt-get install libasio-dev`<br>
Install FLTK: `sudo apt-get install libfltk1.3-dev`<br>

The Raspberry Pi 2 and 3 should work out of the box. When using a Raspberry Pi 1 you must change a function argument in the [timer.cpp](/timer.cpp#L18) from ```ST_BASE_RPI_3``` to ```ST_BASE_RPI_1``` or the system timer will read at the wrong address.

Build the program with ```make``` and build the nmea parser test with ```make test```.

Running the program
---
Simply start the programm with ```sudo ./pipoint```. The program needs root for accessing the MHz [system timer](/timer.cpp) but works without it; All relating values (e.g. system delay) will then be zero.

Acknowledgements / Licenses
---
PiPoint is using [FLTK](http://www.fltk.org) for the GUI and [doctest](https://github.com/onqtam/doctest) for testing.