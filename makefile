# I need to look into writing proper makefiles... tomorrow

CXX=g++-4.9
CXXFLAGS=-std=c++14 -O3 -Wall


CXXFLAGSFLTK = `fltk-config --cxxflags`
LINKFLTK = -lpthread `fltk-config --ldstaticflags`


all: pichi

test: nmea_parser.o test/nmea_parser_test.cpp
	$(CXX) $(CXXFLAGS) nmea_parser.o test/nmea_parser_test.cpp -o nmea_parser_test


pichi: \
  main.o pichi.o mainwindow.o mainwindow_impl.o logfile.o \
  gnss_util.o gnss_receiver.o udp_async_receiver.o udp_transmitter.o udp_base.o \
  nmea_reader.o serial_async_reader.o serial_base.o nmea_parser.o \
  timer.o configuration.o
	$(CXX) $(CXXFLAGS) $(CXXFLAGSFLTK) \
      main.o pichi.o mainwindow.o mainwindow_impl.o logfile.o \
      gnss_util.o gnss_receiver.o udp_async_receiver.o udp_transmitter.o udp_base.o \
      nmea_reader.o serial_async_reader.o serial_base.o nmea_parser.o \
      timer.o configuration.o \
      -o pichi $(LINKFLTK)
	strip pichi
	@echo "Build finished"

main.o: main.cpp gnss_receiver.h ui/mainwindow.h
	$(CXX) -c $(CXXFLAGS) main.cpp

mainwindow.o: ui/mainwindow.cpp
	$(CXX) -c $(CXXFLAGS) $(CXXFLAGSFLTK) ui/mainwindow.cpp

mainwindow_impl.o: ui/mainwindow_impl.cpp gnss_receiver.h configuration.h
	$(CXX) -c $(CXXFLAGS) $(CXXFLAGSFLTK) ui/mainwindow_impl.cpp

pichi.o: \
  pichi.cpp \
  timer.h \
  configuration.h \
  nmea_reader.h \
  nmea_parser.h \
  gnss_receiver.h \
  gnss_util.h
	$(CXX) -c $(CXXFLAGS) pichi.cpp

logfile.o: logfile.cpp logfile.h
	$(CXX) -c $(CXXFLAGS) logfile.cpp

gnss_receiver.o: \
  gnss_receiver.cpp \
  gnss_receiver.h \
  base/udp_async_receiver.h \
  timer.h \
  configuration.h
	$(CXX) -c $(CXXFLAGS) gnss_receiver.cpp

gnss_util.o: gnss_util.cpp gnss_util.h
	$(CXX) -c $(CXXFLAGS) gnss_util.cpp

nmea_reader.o: \
  nmea_reader.cpp \
  nmea_reader.h \
  nmea_parser.h \
  base/serial_async_reader.h \
  timer.h \
  configuration.h
	$(CXX) -c $(CXXFLAGS) nmea_reader.cpp

nmea_parser.o: nmea_parser.cpp nmea_parser.h
	$(CXX) -c $(CXXFLAGS) nmea_parser.cpp

serial_async_reader.o: \
  base/serial_async_reader.cpp base/serial_async_reader.h base/serial_base.h
	$(CXX) -c $(CXXFLAGS) base/serial_async_reader.cpp

serial_base.o: base/serial_base.cpp base/serial_base.h
	$(CXX) -c $(CXXFLAGS) base/serial_base.cpp

udp_async_receiver.o: \
  base/udp_async_receiver.cpp base/udp_async_receiver.h base/udp_base.h
	$(CXX) -c $(CXXFLAGS) base/udp_async_receiver.cpp

udp_transmitter.o: \
  base/udp_transmitter.cpp base/udp_transmitter.h base/udp_base.h
	$(CXX) -c $(CXXFLAGS) base/udp_transmitter.cpp

udp_base.o: base/udp_base.cpp base/udp_base.h
	$(CXX) -c $(CXXFLAGS) base/udp_base.cpp

timer.o: timer.cpp timer.h
	$(CXX) -c $(CXXFLAGS) timer.cpp

configuration.o: configuration.cpp configuration.h
	$(CXX) -c $(CXXFLAGS) configuration.cpp


clean:
	-rm *o pichi
