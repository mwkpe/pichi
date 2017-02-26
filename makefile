# I need to look into writing proper makefiles... tomorrow

CXX=g++-4.9
CXXFLAGS=-std=c++14 -O3 -Wall


CXXFLAGSFLTK = `fltk-config --cxxflags`
LINKFLTK = -lpthread `fltk-config --ldstaticflags`


all: pichi

test: nmea_parser.o src/test/nmea_parser_test.cpp
	$(CXX) $(CXXFLAGS) nmea_parser.o src/test/nmea_parser_test.cpp -o nmea_parser_test


pichi: \
  format.o main.o pichi.o mainwindow.o mainwindow_impl.o log_file.o csv_file.o gpx_file.o \
  util.o receiver.o async_udp_receiver.o udp_transmitter.o udp_base.o udp_socket_opener.o \
  nmea_reader.o async_serial_reader.o serial_port_opener.o nmea_parser.o \
  timer.o configuration.o
	$(CXX) $(CXXFLAGS) $(CXXFLAGSFLTK) \
      format.o main.o pichi.o mainwindow.o mainwindow_impl.o log_file.o csv_file.o gpx_file.o \
      util.o receiver.o async_udp_receiver.o udp_transmitter.o udp_base.o udp_socket_opener.o \
      nmea_reader.o async_serial_reader.o serial_port_opener.o nmea_parser.o \
      timer.o configuration.o \
      -o pichi $(LINKFLTK)
	strip pichi
	@echo "Build finished"

main.o: src/main.cpp src/pichi/receiver.h src/ui/mainwindow.h
	$(CXX) -c $(CXXFLAGS) src/main.cpp

mainwindow.o: src/ui/mainwindow.cpp
	$(CXX) -c $(CXXFLAGS) $(CXXFLAGSFLTK) src/ui/mainwindow.cpp

mainwindow_impl.o: src/ui/mainwindow_impl.cpp src/pichi/receiver.h src/pichi/configuration.h
	$(CXX) -c $(CXXFLAGS) $(CXXFLAGSFLTK) src/ui/mainwindow_impl.cpp

pichi.o: \
  src/pichi/pichi.cpp \
  src/pichi/pichi.h \
  src/util/timer.h \
  src/pichi/configuration.h \
  src/nmea/nmea_reader.h \
  src/nmea/nmea_parser.h \
  src/pichi/receiver.h \
  src/util/util.h
	$(CXX) -c $(CXXFLAGS) src/pichi/pichi.cpp

gpx_file.o: src/log/gpx_file.cpp src/log/gpx_file.h src/base/log_file.h src/ext/fmt/format.h
	$(CXX) -c $(CXXFLAGS) src/log/gpx_file.cpp

csv_file.o: src/log/csv_file.cpp src/log/csv_file.h src/base/log_file.h
	$(CXX) -c $(CXXFLAGS) src/log/csv_file.cpp

log_file.o: src/base/log_file.cpp src/base/log_file.h
	$(CXX) -c $(CXXFLAGS) src/base/log_file.cpp

receiver.o: \
  src/pichi/receiver.cpp \
  src/pichi/receiver.h \
  src/base/async_udp_receiver.h \
  src/util/timer.h \
  src/pichi/configuration.h
	$(CXX) -c $(CXXFLAGS) src/pichi/receiver.cpp

nmea_reader.o: \
  src/nmea/nmea_reader.cpp \
  src/nmea/nmea_reader.h \
  src/nmea/nmea_parser.h \
  src/base/async_serial_reader.h \
  src/util/timer.h \
  src/pichi/configuration.h
	$(CXX) -c $(CXXFLAGS) src/nmea/nmea_reader.cpp

nmea_parser.o: src/nmea/nmea_parser.cpp src/nmea/nmea_parser.h
	$(CXX) -c $(CXXFLAGS) src/nmea/nmea_parser.cpp

async_serial_reader.o: \
  src/base/async_serial_reader.cpp src/base/async_serial_reader.h src/base/serial_port_opener.h
	$(CXX) -c $(CXXFLAGS) src/base/async_serial_reader.cpp

serial_port_opener.o: src/base/serial_port_opener.cpp src/base/serial_port_opener.h
	$(CXX) -c $(CXXFLAGS) src/base/serial_port_opener.cpp

async_udp_receiver.o: \
  src/base/async_udp_receiver.cpp src/base/async_udp_receiver.h src/base/udp_base.h src/base/udp_socket_opener.h
	$(CXX) -c $(CXXFLAGS) src/base/async_udp_receiver.cpp

udp_transmitter.o: \
  src/base/udp_transmitter.cpp src/base/udp_transmitter.h src/base/udp_base.h src/base/udp_socket_opener.h
	$(CXX) -c $(CXXFLAGS) src/base/udp_transmitter.cpp

udp_socket_opener.o: src/base/udp_socket_opener.cpp src/base/udp_socket_opener.h
	$(CXX) -c $(CXXFLAGS) src/base/udp_socket_opener.cpp

udp_base.o: src/base/udp_base.cpp src/base/udp_base.h
	$(CXX) -c $(CXXFLAGS) src/base/udp_base.cpp

timer.o: src/util/timer.cpp src/util/timer.h
	$(CXX) -c $(CXXFLAGS) src/util/timer.cpp

configuration.o: src/pichi/configuration.cpp src/pichi/configuration.h
	$(CXX) -c $(CXXFLAGS) src/pichi/configuration.cpp

util.o: src/util/util.cpp src/util/util.h
	$(CXX) -c $(CXXFLAGS) src/util/util.cpp

format.o: src/ext/fmt/format.cc src/ext/fmt/format.h
	$(CXX) -c $(CXXFLAGS) src/ext/fmt/format.cc


clean:
	-rm *o pichi
