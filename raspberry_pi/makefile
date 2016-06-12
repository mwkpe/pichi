CXX=g++-4.9
CXXFLAGS=-std=c++14 -O3 -Wall


CXXFLAGSFLTK = `fltk-config --cxxflags`
LINKFLTK = -lpthread `fltk-config --ldstaticflags`


all: pipoint

test: nmea_parser.o nmea_parser_test.cpp
	$(CXX) $(CXXFLAGS) nmea_parser.o nmea_parser_test.cpp -o nmea_parser_test


pipoint: main.o ui.o ui_impl.o pipoint.o gnss_transceiver.o nmea_reader.o \
nmea_parser.o udp_transceiver.o serial_reader.o timer.o configuration.o
	$(CXX) $(CXXFLAGS) $(CXXFLAGSFLTK) main.o ui.o ui_impl.o pipoint.o gnss_transceiver.o \
nmea_reader.o nmea_parser.o udp_transceiver.o serial_reader.o timer.o \
configuration.o -o pipoint $(LINKFLTK)
	strip pipoint

main.o: main.cpp gnss_transceiver.h ui.h
	$(CXX) -c $(CXXFLAGS) main.cpp

ui.o: ui.cpp
	$(CXX) -c $(CXXFLAGS) $(CXXFLAGSFLTK) ui.cpp

ui_impl.o: ui_impl.cpp gnss_transceiver.h configuration.h
	$(CXX) -c $(CXXFLAGS) $(CXXFLAGSFLTK) ui_impl.cpp

pipoint.o: pipoint.cpp pipoint.h gnss_transceiver.h nmea_reader.h timer.h configuration.h
	$(CXX) -c $(CXXFLAGS) pipoint.cpp

gnss_transceiver.o: gnss_transceiver.cpp gnss_transceiver.h udp_transceiver.h timer.h configuration.h
	$(CXX) -c $(CXXFLAGS) gnss_transceiver.cpp

nmea_reader.o: nmea_reader.cpp nmea_reader.h serial_reader.h timer.h configuration.h
	$(CXX) -c $(CXXFLAGS) nmea_reader.cpp

nmea_parser.o: nmea_parser.cpp nmea_parser.h
	$(CXX) -c $(CXXFLAGS) nmea_parser.cpp

serial_reader.o: serial_reader.cpp serial_reader.h
	$(CXX) -c $(CXXFLAGS) serial_reader.cpp

udp_transceiver.o: udp_transceiver.cpp udp_transceiver.h
	$(CXX) -c $(CXXFLAGS) udp_transceiver.cpp

timer.o: timer.cpp timer.h
	$(CXX) -c $(CXXFLAGS) timer.cpp

configuration.o: configuration.cpp configuration.h
	$(CXX) -c $(CXXFLAGS) configuration.cpp


clean:
	-rm *o pipoint
