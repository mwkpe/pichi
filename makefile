# I need to look into writing proper makefiles... tomorrow

CXX=g++-4.9
CXXFLAGS=-std=c++14 -O3 -Wall


CXXFLAGSFLTK = `fltk-config --cxxflags`
LINKFLTK = -lpthread `fltk-config --ldstaticflags`


all: pichi

test: nmea_parser.o src/test/nmea_parser_test.cpp
	$(CXX) $(CXXFLAGS) nmea_parser.o src/test/nmea_parser_test.cpp -o nmea_parser_test


pichi: \
  main.o pichi.o mainwindow.o mainwindow_impl.o logfile.o \
  util.o gnss_receiver.o udp_async_receiver.o udp_transmitter.o udp_base.o \
  nmea_reader.o serial_async_reader.o serial_base.o nmea_parser.o \
  timer.o configuration.o
	$(CXX) $(CXXFLAGS) $(CXXFLAGSFLTK) \
      main.o pichi.o mainwindow.o mainwindow_impl.o logfile.o \
      util.o gnss_receiver.o udp_async_receiver.o udp_transmitter.o udp_base.o \
      nmea_reader.o serial_async_reader.o serial_base.o nmea_parser.o \
      timer.o configuration.o \
      -o pichi $(LINKFLTK)
	strip pichi
	@echo "Build finished"

main.o: src/main.cpp src/gnss/receiver.h src/ui/mainwindow.h
	$(CXX) -c $(CXXFLAGS) src/main.cpp

mainwindow.o: src/ui/mainwindow.cpp
	$(CXX) -c $(CXXFLAGS) $(CXXFLAGSFLTK) src/ui/mainwindow.cpp

mainwindow_impl.o: src/ui/mainwindow_impl.cpp src/gnss/receiver.h src/configuration.h
	$(CXX) -c $(CXXFLAGS) $(CXXFLAGSFLTK) src/ui/mainwindow_impl.cpp

pichi.o: \
  src/pichi.cpp \
  src/timer.h \
  src/configuration.h \
  src/nmea/reader.h \
  src/nmea/parser.h \
  src/gnss/receiver.h \
  src/util/util.h
	$(CXX) -c $(CXXFLAGS) src/pichi.cpp

logfile.o: src/logfile.cpp src/logfile.h
	$(CXX) -c $(CXXFLAGS) src/logfile.cpp

gnss_receiver.o: \
  src/gnss/receiver.cpp \
  src/gnss/receiver.h \
  src/base/udp_async_receiver.h \
  src/timer.h \
  src/configuration.h
	$(CXX) -c $(CXXFLAGS) src/gnss/receiver.cpp -o gnss_receiver.o

nmea_reader.o: \
  src/nmea/reader.cpp \
  src/nmea/reader.h \
  src/nmea/parser.h \
  src/base/serial_async_reader.h \
  src/timer.h \
  src/configuration.h
	$(CXX) -c $(CXXFLAGS) src/nmea/reader.cpp -o nmea_reader.o

nmea_parser.o: src/nmea/parser.cpp src/nmea/parser.h
	$(CXX) -c $(CXXFLAGS) src/nmea/parser.cpp -o nmea_parser.o

serial_async_reader.o: \
  src/base/serial_async_reader.cpp src/base/serial_async_reader.h src/base/serial_base.h
	$(CXX) -c $(CXXFLAGS) src/base/serial_async_reader.cpp

serial_base.o: src/base/serial_base.cpp src/base/serial_base.h
	$(CXX) -c $(CXXFLAGS) src/base/serial_base.cpp

udp_async_receiver.o: \
  src/base/udp_async_receiver.cpp src/base/udp_async_receiver.h src/base/udp_base.h
	$(CXX) -c $(CXXFLAGS) src/base/udp_async_receiver.cpp

udp_transmitter.o: \
  src/base/udp_transmitter.cpp src/base/udp_transmitter.h src/base/udp_base.h
	$(CXX) -c $(CXXFLAGS) src/base/udp_transmitter.cpp

udp_base.o: src/base/udp_base.cpp src/base/udp_base.h
	$(CXX) -c $(CXXFLAGS) src/base/udp_base.cpp

timer.o: src/timer.cpp src/timer.h
	$(CXX) -c $(CXXFLAGS) src/timer.cpp

configuration.o: src/configuration.cpp src/configuration.h
	$(CXX) -c $(CXXFLAGS) src/configuration.cpp

util.o: src/util/util.cpp src/util/util.h
	$(CXX) -c $(CXXFLAGS) src/util/util.cpp


clean:
	-rm *o pichi
