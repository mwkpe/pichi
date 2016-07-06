Build setup
---
Compilation requires GCC 4.9 (or later with updating the makefile), boost (Spirit), FLTK 1.3 and the non-boost version of Asio. The first two should be part of any recent Raspian.

Install Asio: `sudo apt-get install libasio-dev`<br>
Install FLTK: `sudo apt-get install libfltk1.3-dev`<br>

The Raspberry Pi 2 and 3 should work out of the box. When using a Raspberry Pi 1 a function argument in the [timer.cpp](src/timer.cpp#L20) must be changed from `ST_BASE_RPI_2_AND_3` to `ST_BASE_RPI_1_AND_ZERO` or the system timer will read at the wrong address.

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
