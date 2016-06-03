#ifndef NMEA_PARSER_H
#define NMEA_PARSER_H


/* A simple parser for NMEA sentences (RMC, GGA, GSV)

   Important: This parser is not based on the official NMEA 0183 interface
              standard specification (sold by the NME Association). It makes
              no claims of being correct or complete. Not all sentence versions
              may work.

   Usage: Sentences must begin with an $ and should end with *hh, though any
          additional characters (e.g. <CR><LF>) are ignored in all functions.
*/


#include <cstdint>
#include <string>
#include <tuple>


namespace nmea {


// Sentence type deduction
enum class SentenceType { Unknown, Rmc, Gga, Gsv };
SentenceType sentence_type(const std::string& sentence);


// Recommended minimum specific GPS/Transit data
struct RmcData
{
  char talker_id;
  uint8_t utc_time_hour;
  uint8_t utc_time_minute;
  float utc_time_second;
  char status;
  int16_t degrees_lat;
  float minutes_lat;
  char direction_lat;
  int16_t degrees_long;
  float minutes_long;
  char direction_long;
  float speed_over_ground;
  float track_angle;
  uint8_t date_day;
  uint8_t date_month;
  uint8_t date_year;
  float magnetic_variation;
  char direction_mv;
  char mode_indicator;
};

bool parse(const std::string& sentence, RmcData* data, uint8_t* checksum);


// Global positioning system fix data
struct GgaData
{
  char talker_id;
  uint8_t utc_time_hour;
  uint8_t utc_time_minute;
  float utc_time_second;
  int16_t degrees_lat;
  float minutes_lat;
  char direction_lat;
  int16_t degrees_long;
  float minutes_long;
  char direction_long;
  uint8_t fix_flag;
  uint8_t satellites_used;
  float hor_dilution_of_precision;
  float altitude;
  float geoidal_separation;
  float age_of_dgps_data;
  uint16_t reference_station_id;
};

bool parse(const std::string& sentence, GgaData* data, uint8_t* checksum);


// GPS Satellites in view
struct Satellite
{
  bool has_data;
  uint16_t satellite_prn_number;
  uint8_t elevation;
  uint16_t azimuth;
  uint8_t snr;
};

struct GsvData
{
  char talker_id;
  uint8_t total_messages;
  uint8_t message_number;
  uint8_t satellites_in_view;
  Satellite first;
  Satellite second;
  Satellite third;
  Satellite fourth;
};

bool parse(const std::string& sentence, GsvData* data, uint8_t* checksum);


// Additional functions
template<typename T> std::tuple<bool, T, uint8_t> parse(const std::string& sentence)
{
  T data;
  uint8_t checksum;
  bool success = parse(sentence, &data, &checksum);
  return std::make_tuple(success, data, checksum);
}

uint8_t calc_checksum(const std::string& s, bool* ok = nullptr);

inline bool comp_checksum(const std::string& s, uint8_t checksum)
{
  bool ok;
  return calc_checksum(s, &ok) == checksum && ok;
}


}  // nmea


#endif  // NMEA_PARSER_H
