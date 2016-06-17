#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"


#include "../../nmea_parser.h"


using namespace nmea;
using doctest::Approx;


TEST_CASE("GxRMC parser") {
  RmcData data;
  uint8_t crc;

  // From NL-8002U NAVILOCK Multi GNSS Receiver ublox 8
  CHECK(parse("$GNRMC,031649.50,A,4958.42331,N,00909.23445,E,1.039,,250516,,,A*6F", &data, &crc) == true);

  CHECK(data.talker_id == 'N');
  CHECK(data.utc_time_hour == 3);
  CHECK(data.utc_time_minute == 16);
  CHECK(Approx(data.utc_time_second) == 49.5);
  CHECK(data.status == 'A');
  CHECK(data.degrees_lat == 49);
  CHECK(Approx(data.minutes_lat) == 58.42331);
  CHECK(data.direction_lat == 'N');
  CHECK(data.degrees_long == 9);
  CHECK(Approx(data.minutes_long) == 9.23445);
  CHECK(data.direction_long == 'E');
  CHECK(Approx(data.speed_over_ground) == 1.039);
  CHECK(Approx(data.track_angle) == 0);
  CHECK(data.date_day == 25);
  CHECK(data.date_month == 5);
  CHECK(data.date_year == 16);
  CHECK(data.magnetic_variation == 0);
  CHECK(data.direction_mv == '0');
  CHECK(data.mode_indicator == 'A');
  

  // Generic

  // NMEA 2.3 (and later) examples
  CHECK(parse("$GPRMC,052848,A,4958.43074,N,00909.25157,E,0.371,,240516,,,A*5C", &data, &crc) == true);
  CHECK(parse("$GNRMC,052848.40,A,4958.43074,N,00909.25157,E,0.371,0.451,240516,,,D*43", &data, &crc) == true);
  CHECK(parse("$GLRMC,052848.40,V,4958.43074,S,00909.25157,W,,,240516,,,E*5D", &data, &crc) == true);
  CHECK(parse("$GARMC,052848.40,A,4958.43074,N,00909.25157,E,,0.451,240516,0.332,E,N*04", &data, &crc) == true);
  CHECK(parse("$GBRMC,052848.40,A,4958.43074,N,00909.25157,E,0.371,,240516,,,S*76", &data, &crc) == true);
  CHECK(parse("$GPRMC,052848,A,4958.43074,N,00909.25157,E,0.371,,240516,,,F*5B", &data, &crc) == true);
  CHECK(parse("$GPRMC,052848,A,4958.43074,N,00909.25157,E,0.371,,240516,,,R*4F", &data, &crc) == true);

  // NMEA pre 2.3 (no mode indicator)
  CHECK(parse("$GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70", &data, &crc) == true);
}


TEST_CASE("GxGGA parser") {
  GgaData data;
  uint8_t crc;

  // From NL-8002U NAVILOCK Multi GNSS Receiver ublox 8
  CHECK(parse("$GNGGA,031649.50,4958.42331,N,00909.23445,E,1,09,1.08,156.1,M,47.7,M,,*4A", &data, &crc) == true);

  CHECK(data.talker_id == 'N');
  CHECK(data.utc_time_hour == 3);
  CHECK(data.utc_time_minute == 16);
  CHECK(Approx(data.utc_time_second) == 49.5);
  CHECK(data.degrees_lat == 49);
  CHECK(Approx(data.minutes_lat) == 58.42331);
  CHECK(data.direction_lat == 'N');
  CHECK(data.degrees_long == 9);
  CHECK(Approx(data.minutes_long) == 9.23445);
  CHECK(data.direction_long == 'E');
  CHECK(data.fix_flag == 1);
  CHECK(data.satellites_used == 9);
  CHECK(Approx(data.hor_dilution_of_precision) == 1.08);
  CHECK(Approx(data.altitude) == 156.1);
  CHECK(Approx(data.geoidal_separation) == 47.7);
  CHECK(Approx(data.age_of_dgps_data) == 0);
  CHECK(data.reference_station_id == 0);


  // Generic
  CHECK(parse("$GPGGA,152835.00,5554.0114,N,03732.5007,E,1,13,00.8,170.4,M,14.5,M,,*5E", &data, &crc) == true);
  CHECK(parse("$GNGGA,150947.00,5554.0083,N,03732.502,E,1,15,00.6,190.6,M,14.5,M,,*78", &data, &crc) == true);
  CHECK(parse("$GPGGA,151114.00,5554.0093,N,03732.5027,E,1,11,00.7,196.4,M,14.5,M,,*5E", &data, &crc) == true);
  CHECK(parse("$GLGGA,150626.00,5554.0097,N,03732.4979,E,1,06,01.2,192.6,M,14.5,M,,*46", &data, &crc) == true);
}


TEST_CASE("GxGSV parser") {
  GsvData data;
  uint8_t crc;

  // From NL-8002U NAVILOCK Multi GNSS Receiver ublox 8

  // GPS satellites

  // Check first message (1/3)
  CHECK(parse("$GPGSV,3,1,12,02,36,299,24,03,17,119,10,05,05,298,,06,51,230,23*77", &data, &crc) == true);
  CHECK(data.talker_id == 'P');
  CHECK(data.total_messages == 3);
  CHECK(data.message_number == 1);
  CHECK(data.satellites_in_view == 12);
    CHECK(data.first.has_data == true);
    CHECK(data.first.satellite_prn_number == 2);
    CHECK(data.first.elevation == 36);
    CHECK(data.first.azimuth == 299);
    CHECK(data.first.snr == 24);

    CHECK(data.second.has_data == true);
    CHECK(data.second.satellite_prn_number == 3);
    CHECK(data.second.elevation == 17);
    CHECK(data.second.azimuth == 119);
    CHECK(data.second.snr == 10);

    CHECK(data.third.has_data == true);
    CHECK(data.third.satellite_prn_number == 5);
    CHECK(data.third.elevation == 5);
    CHECK(data.third.azimuth == 298);
    CHECK(data.third.snr == 0);

    CHECK(data.fourth.has_data == true);
    CHECK(data.fourth.satellite_prn_number == 6);
    CHECK(data.fourth.elevation == 51);
    CHECK(data.fourth.azimuth == 230);
    CHECK(data.fourth.snr == 23);
  CHECK(crc == 0x77);

  // Check second message (2/3)
  CHECK(parse("$GPGSV,3,2,12,07,42,173,22,09,81,053,16,16,15,069,,19,00,233,*78", &data, &crc) == true);
  CHECK(data.talker_id == 'P');
  CHECK(data.total_messages == 3);
  CHECK(data.message_number == 2);
  CHECK(data.satellites_in_view == 12);
    CHECK(data.first.has_data == true);
    CHECK(data.first.satellite_prn_number == 7);
    CHECK(data.first.elevation == 42);
    CHECK(data.first.azimuth == 173);
    CHECK(data.first.snr == 22);

    CHECK(data.second.has_data == true);
    CHECK(data.second.satellite_prn_number == 9);
    CHECK(data.second.elevation == 81);
    CHECK(data.second.azimuth == 53);
    CHECK(data.second.snr == 16);

    CHECK(data.third.has_data == true);
    CHECK(data.third.satellite_prn_number == 16);
    CHECK(data.third.elevation == 15);
    CHECK(data.third.azimuth == 69);
    CHECK(data.third.snr == 0);

    CHECK(data.fourth.has_data == true);
    CHECK(data.fourth.satellite_prn_number == 19);
    CHECK(data.fourth.elevation == 0);
    CHECK(data.fourth.azimuth == 233);
    CHECK(data.fourth.snr == 0);
  CHECK(crc == 0x78);

  // Check third message (3/3)
  CHECK(parse("$GPGSV,3,3,12,23,49,068,19,26,12,041,,29,04,345,,30,17,190,18*7B", &data, &crc) == true);
  CHECK(data.talker_id == 'P');
  CHECK(data.total_messages == 3);
  CHECK(data.message_number == 3);
  CHECK(data.satellites_in_view == 12);
    CHECK(data.first.has_data == true);
    CHECK(data.first.satellite_prn_number == 23);
    CHECK(data.first.elevation == 49);
    CHECK(data.first.azimuth == 68);
    CHECK(data.first.snr == 19);

    CHECK(data.second.has_data == true);
    CHECK(data.second.satellite_prn_number == 26);
    CHECK(data.second.elevation == 12);
    CHECK(data.second.azimuth == 41);
    CHECK(data.second.snr == 0);

    CHECK(data.third.has_data == true);
    CHECK(data.third.satellite_prn_number == 29);
    CHECK(data.third.elevation == 4);
    CHECK(data.third.azimuth == 345);
    CHECK(data.third.snr == 0);

    CHECK(data.fourth.has_data == true);
    CHECK(data.fourth.satellite_prn_number == 30);
    CHECK(data.fourth.elevation == 17);
    CHECK(data.fourth.azimuth == 190);
    CHECK(data.fourth.snr == 18);
  CHECK(crc == 0x7B);


  // GLONASS satellites
  CHECK(parse("$GLGSV,3,1,09,65,21,293,23,72,05,092,,73,48,073,19,74,36,147,24*6A", &data, &crc) == true);
  CHECK(parse("$GLGSV,3,2,09,80,13,022,,81,06,354,,87,24,250,26,88,30,305,*65", &data, &crc) == true);

  // Check last message (3/3)
  CHECK(parse("$GLGSV,3,3,09,95,13,022,*52", &data, &crc) == true);
  CHECK(data.talker_id == 'L');
  CHECK(data.total_messages == 3);
  CHECK(data.message_number == 3);
  CHECK(data.satellites_in_view == 9);
    CHECK(data.first.has_data == true);
    CHECK(data.first.satellite_prn_number == 95);
    CHECK(data.first.elevation == 13);
    CHECK(data.first.azimuth == 22);
    CHECK(data.first.snr == 0);

    CHECK(data.second.has_data == false);
    CHECK(data.third.has_data == false);
    CHECK(data.fourth.has_data == false);
  CHECK(crc == 0x52);


  // Generic
  CHECK(parse("$GPGSV,3,1,12,11,52,219,48,12,09,021,40,14,34,057,47,17,25,306,45*72", &data, &crc) == true);
  CHECK(parse("$GPGSV,3,2,12,20,46,274,48,23,14,223,45,24,67,214,49,31,35,123,48*75", &data, &crc) == true);
  CHECK(parse("$GPGSV,3,3,12,32,78,266,51,33,11,238,39,37,15,197,37,39,25,195,00*7A", &data, &crc) == true);
  CHECK(parse("$GLGSV,2,1,07,65,36,079,51,66,77,331,53,74,15,014,42,75,41,067,49*65", &data, &crc) == true);
  CHECK(parse("$GLGSV,2,2,07,76,24,132,50,82,41,296,48,83,13,346,43*78", &data, &crc) == true);
}


TEST_CASE("Checksum calculation") {
  CHECK(calc_checksum("$GNRMC,031649.50,A,4958.42331,N,00909.23445,E,1.039,,250516,,,A*6F") == 0x6F);
  CHECK(calc_checksum("$GNGGA,031649.50,4958.42331,N,00909.23445,E,1,09,1.08,156.1,M,47.7,M,,*4A") == 0x4A);
  CHECK(calc_checksum("$GPGSV,3,2,12,07,42,173,22,09,81,053,16,16,15,069,,19,00,233,*78") == 0x78);
}
