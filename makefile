CXX=g++-4.9 -std=c++14 -O3 -Wall

CXXFLAGSFLTK = `fltk-config --cxxflags`
LINKFLTK = -lpthread `fltk-config --ldstaticflags`


all: pipoint

test: nmea_parser.o nmea_parser_test.cpp
	$(CXX) nmea_parser.o nmea_parser_test.cpp -o nmea_parser_test


pipoint: main.o ui.o ui_impl.o gnss_transceiver.o nmea_parser.o timer.o
	$(CXX) $(CXXFLAGSFLTK) main.o ui.o ui_impl.o gnss_transceiver.o \
nmea_parser.o timer.o -o pipoint $(LINKFLTK)
	strip pipoint

main.o: main.cpp gnss_transceiver.h ui.h
	$(CXX) -c main.cpp

ui.o: ui.cpp
	$(CXX) -c $(CXXFLAGSFLTK) ui.cpp

ui_impl.o: ui_impl.cpp
	$(CXX) -c $(CXXFLAGSFLTK) ui_impl.cpp

gnss_transceiver.o: gnss_transceiver.cpp gnss_transceiver.h timer.h
	$(CXX) -c gnss_transceiver.cpp

nmea_parser.o: nmea_parser.cpp nmea_parser.h
	$(CXX) -c nmea_parser.cpp

timer.o: timer.cpp timer.h
	$(CXX) -c timer.cpp


clean:
	-rm *o pipoint