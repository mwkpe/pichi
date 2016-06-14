CXX=g++-4.9
CXXFLAGS=-std=c++14 -O3 -Wall


CXXFLAGSFLTK = `fltk-config --cxxflags`
LINKFLTK = -lpthread `fltk-config --ldstaticflags`


all: pichi

test: nmea_parser.o nmea_parser_test.cpp
	$(CXX) $(CXXFLAGS) nmea_parser.o nmea_parser_test.cpp -o nmea_parser_test


pichi: \
  main.o pichi.o mainwindow.o mainwindow_impl.o \
  gnss_transceiver.o udp_transceiver.o \
  nmea_reader.o serial_reader.o nmea_parser.o \
  timer.o configuration.o
	$(CXX) $(CXXFLAGS) $(CXXFLAGSFLTK) \
      main.o pichi.o mainwindow.o mainwindow_impl.o \
      gnss_transceiver.o udp_transceiver.o \
      nmea_reader.o serial_reader.o nmea_parser.o \
      timer.o configuration.o \
      -o pichi $(LINKFLTK)
	strip pichi


main.o: main.cpp gnss_transceiver.h ui/mainwindow.h
	$(CXX) -c $(CXXFLAGS) main.cpp

mainwindow.o: ui/mainwindow.cpp
	$(CXX) -c $(CXXFLAGS) $(CXXFLAGSFLTK) ui/mainwindow.cpp

mainwindow_impl.o: ui/mainwindow_impl.cpp gnss_transceiver.h configuration.h
	$(CXX) -c $(CXXFLAGS) $(CXXFLAGSFLTK) ui/mainwindow_impl.cpp

pichi.o: pichi.cpp configuration.h
	$(CXX) -c $(CXXFLAGS) pichi.cpp

gnss_transceiver.o: \
  gnss_transceiver.cpp \
  gnss_transceiver.h \
  base/udp_transceiver.h \
  timer.h \
  configuration.h
	$(CXX) -c $(CXXFLAGS) gnss_transceiver.cpp

nmea_reader.o: \
  nmea_reader.cpp \
  nmea_reader.h \
  base/serial_reader.h \
  timer.h \
  configuration.h
	$(CXX) -c $(CXXFLAGS) nmea_reader.cpp

nmea_parser.o: nmea_parser.cpp nmea_parser.h
	$(CXX) -c $(CXXFLAGS) nmea_parser.cpp

serial_reader.o: base/serial_reader.cpp base/serial_reader.h
	$(CXX) -c $(CXXFLAGS) base/serial_reader.cpp

udp_transceiver.o: base/udp_transceiver.cpp base/udp_transceiver.h
	$(CXX) -c $(CXXFLAGS) base/udp_transceiver.cpp

timer.o: timer.cpp timer.h
	$(CXX) -c $(CXXFLAGS) timer.cpp

configuration.o: configuration.cpp configuration.h
	$(CXX) -c $(CXXFLAGS) configuration.cpp


clean:
	-rm *o pichi
