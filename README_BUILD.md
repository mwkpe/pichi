Build setup
---
Compilation requires GCC 4.9 (or later with updating the makefile), boost (Spirit), FLTK 1.3 and the non-boost version of Asio. The first two should be part of any recent Raspian.

Install Asio: `sudo apt-get install libasio-dev`<br>
Install FLTK: `sudo apt-get install libfltk1.3-dev`<br>

Missing dependencies
---
Installing FLTK doesn't seem to grab all the required dependencies and the build process may fail with the following error:
```
/usr/bin/ld: cannot find -lXft
/usr/bin/ld: cannot find -lfontconfig
/usr/bin/ld: cannot find -lfontconfig
/usr/bin/ld: cannot find -lXinerama
```
This can be fixed by manually installing `libxft-dev`, `libfontconfig1-dev` and `libxinerama-dev`.

Swap file size
---
This step is necessary when using an original RPi or RPi Zero. The build process may fail with `virtual memory exhausted: Cannot allocate memory` due to low RAM (<1GB). This can be handled by extending the swap file size. The Raspian distribution's default swap file size is only 100MB and must be changed to 1GB. Use `sudo nano /etc/dphys-swapfile` to open the configuration file, look for the line `CONF_SWAPSIZE=100` and change it to `CONF_SWAPSIZE=1024`.

To apply the changes restart the service managing the swap file:
```
sudo /etc/init.d/dphys-swapfile stop
sudo /etc/init.d/dphys-swapfile start
```
The swap file size can be checked with `free -m`. Don't forget to change it back to preserve the SD card.

Set timer address
---
The Raspberry Pi 2 and 3 should work out of the box. When using a Raspberry Pi 1 or Zero a function argument in the [timer.cpp](src/timer.cpp#L20) must be changed from `ST_BASE_RPI_2_AND_3` to `ST_BASE_RPI_1_AND_ZERO` or the system timer will read at the wrong address.

Build program
---
Navigate to the directory containing the makefile and src directory. Build the program with `make` (or `make -j6` when using a Pi 2 or 3). The build process may take anywhere from a few minutes to an half an hour depending on the device. The NMEA parser test can be build with `make test`.

Testing
---
Use `./nmea_parser_test` to run the test of the NMEA parser.

Navigate to `/src/test` and use `./run_cpplint.sh` to run cpplint. Remember that both scripts, the run_cpplint shell script and the cpplint python script in `src/ext` require execute rights.
