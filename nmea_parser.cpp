#include "nmea_parser.h"


#include <boost/spirit/include/qi.hpp>
#include <vector>
#include <iterator>
#include <numeric>
#include <algorithm>


// Sentence type deduction
nmea::SentenceType nmea::sentence_type(const std::string& sentence)
{
  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;

  using qi::phrase_parse;
  using qi::lit;
  using qi::omit;
  using ascii::space;
  using ascii::char_;

  std::string message_id{};

  // $aaXXX,... (XXX = Message ID)
  // $GPRMC,...

  phrase_parse(std::begin(sentence), std::end(sentence),
    lit("$G") >> omit[char_] >> +char_('A', 'Z') >> ',', space, message_id);

  if (message_id == "RMC") { return SentenceType::Rmc; }
  if (message_id == "GGA") { return SentenceType::Gga; }
  if (message_id == "GSV") { return SentenceType::Gsv; }

  return SentenceType::Unknown;
}


// Recommended minimum specific GPS/Transit data
bool nmea::parse(const std::string& sentence,
                 gsl::not_null<RmcData*> data,
                 gsl::not_null<uint8_t*> checksum)
{
  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;

  using qi::phrase_parse;
  using qi::lit;
  using qi::attr;
  using qi::uint_parser;
  using qi::int_parser;
  using qi::ulong_;
  using qi::ushort_;
  using qi::float_;
  using qi::hex;
  using ascii::space;
  using ascii::char_;

  // Custom parser
  auto uint8_2d = uint_parser<uint8_t, 10, 2, 2>{};
  auto uint16_3d = uint_parser<uint16_t, 10, 3, 3>{};

  // Temporary variables
  boost::optional<char> mode_indicator;

  // $aaRMC,hhmmss.s-s,A,llll.l-l,a,yyyyy.y-y,a,x.x,x.x,ddmmyy,x.x,a,a*hh
  // $GPRMC,054100.00,A,5552.9539,N,03727.3206,E,40.13,087.9,250211,,,A*5C

  auto it = std::begin(sentence);
  bool success = phrase_parse(it, std::end(sentence),
    lit("$") >>               // Start character ($)
    'G' >> char_("PNLAB") >>  // Talker ID (aa)
    "RMC," >>                 // Message id (RMC)
    uint8_2d >>               // Time of position fix (hh)
    uint8_2d >>               // Time of position fix (mm)
    float_ >> ',' >>          // Time of position fix (ss.s-s)
    char_("AV") >> ',',       // Status (A)
    space,
    data->talker_id,
    data->utc_time_hour,
    data->utc_time_minute,
    data->utc_time_second,
    data->status)

  &&

  phrase_parse(it, std::end(sentence),
    uint8_2d >> float_ >>            // Latitude (llll.l-l)
    ',' >> char_("NS") >> ',' >>     // Direction (a)
    uint16_3d >> float_ >>           // Longitude (yyyyy.y-y)
    ',' >> char_("EW") >> ',' >>     // Direction (a)
    (float_ | attr(0.0f)) >> ',' >>  // Speed over ground (x.x)
    (float_ | attr(0.0f)) >> ',',    // Course over ground (x.x)
    space,
    data->degrees_lat,
    data->minutes_lat,
    data->direction_lat,
    data->degrees_long,
    data->minutes_long,
    data->direction_long,
    data->speed_over_ground,
    data->track_angle)

  &&

  phrase_parse(it, std::end(sentence),
    uint8_2d >> uint8_2d >> uint8_2d >> ',' >>  // Date (ddmmyy)
    (float_ | attr(0.0f)) >> ',' >>             // Magnetic variation (x.x)
    (char_("EW") | attr('0')) >>                // Direction (a)
    -(',' >> char_("ADENSFR")) >>               // Mode indicator (a)
    '*' >> hex,                                 // Checksum indicator and checksum
    space,
    data->date_day,
    data->date_month,
    data->date_year,
    data->magnetic_variation,
    data->direction_mv,
    mode_indicator,
    *checksum);

  if (success) {
    if (mode_indicator)
      data->mode_indicator = *mode_indicator;
    else
      data->mode_indicator = '0';
  }

  return success;
}


// Global positioning system fix data
bool nmea::parse(const std::string& sentence,
                 gsl::not_null<GgaData*> data,
                 gsl::not_null<uint8_t*> checksum)
{
  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;

  using qi::phrase_parse;
  using qi::lit;
  using qi::attr;
  using qi::uint_parser;
  using qi::int_parser;
  using qi::ulong_;
  using qi::ushort_;
  using qi::float_;
  using qi::hex;
  using ascii::space;
  using ascii::char_;

  // Custom parser
  auto uchar_ = uint_parser<uint8_t>{};
  auto uint8_2d = uint_parser<uint8_t, 10, 2, 2>{};
  auto uint16_3d = uint_parser<uint16_t, 10, 3, 3>{};

  // $aaGGA,hhmmss.s-s,llll.l-l,a,yyyyy.y-y,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
  // $GNGGA,150947.00,5554.0083,N,03732.502,E,1,15,00.6,190.6,M,14.5,M,,*78

  auto it = std::begin(sentence);
  bool success = phrase_parse(it, std::end(sentence),
    lit("$") >>               // Start character ($)
    'G' >> char_("PNLAB") >>  // Talker ID (aa)
    "GGA," >>                 // Message id (GGA)
    uint8_2d >>               // Time of position fix (hh)
    uint8_2d >>               // Time of position fix (mm)
    float_ >> ',',            // Time of position fix (ss.s-s)
    space,
    data->talker_id,
    data->utc_time_hour,
    data->utc_time_minute,
    data->utc_time_second)

  &&

  phrase_parse(it, std::end(sentence),
    uint8_2d >> float_ >> ',' >>   // Latitude (llll.l-l)
    char_("NS") >> ',' >>          // Direction (a)
    uint16_3d >> float_ >> ',' >>  // Longitude (yyyyy.y-y)
    char_("EW") >> ',' >>          // Direction (a)
    uchar_ >> ',' >>               // Position fix flag (x)
    uchar_ >> ',',                 // Number of satellites used in calculation (xx)
    space,
    data->degrees_lat,
    data->minutes_lat,
    data->direction_lat,
    data->degrees_long,
    data->minutes_long,
    data->direction_long,
    data->fix_flag,
    data->satellites_used)

  &&

  phrase_parse(it, std::end(sentence),
    float_ >> ',' >>                 // Horizontal dilution of precision (x.x)
    float_ >> ',' >> 'M' >> ',' >>   // Altitude (x.x,M)
    float_ >> ',' >> 'M' >> ',' >>   // Geoidal separation (x.x,M)
    (float_ | attr(0.0f)) >> ',' >>  // Age of differential GNSS data
    (ushort_ | attr(0)) >>           // Differential reference station ID
    '*' >> hex,                      // Checksum indicator and checksum
    space,
    data->hor_dilution_of_precision,
    data->altitude,
    data->geoidal_separation,
    data->age_of_dgps_data,
    data->reference_station_id,
    *checksum
  );

  return success;
}


// GPS Satellites in view
bool nmea::parse(const std::string& sentence,
                 gsl::not_null<GsvData*> data,
                 gsl::not_null<uint8_t*> checksum)
{
  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;

  using qi::phrase_parse;
  using qi::lit;
  using qi::uint_parser;
  using qi::ushort_;
  using qi::hex;
  using ascii::space;
  using ascii::char_;

  // Custom parser
  auto uchar_ = uint_parser<uint8_t>{};

  // Temporary variables
  std::vector<boost::optional<uint16_t>> sat_data;

  // $aaGSV,x,x,xx,xx,xx,xxx,xx,xx...,xx*hh
  // $GPGSV,3,1,12,11,52,219,48,12,09,021,40,14,34,057,47,17,25,306,45*72

  auto it = std::begin(sentence);
  bool success = phrase_parse(it, std::end(sentence),
    lit("$") >>               // Start character ($)
    'G' >> char_("PNLAB") >>  // Talker ID (aa)
    "GSV," >>                 // Message id (GSV)
    uchar_ >> ',' >>          // Total number of messages
    uchar_ >> ',' >>          // Message number
    uchar_ >> ',' >>          // Total number of satellites in view
    -ushort_ % ',' >>         // PRN, elevation, azimuth and SNR for up to 4 satellites
    '*' >> hex,               // Checksum indicator and checksum
    space,
    data->talker_id,
    data->total_messages,
    data->message_number,
    data->satellites_in_view,
    sat_data,
    *checksum
  );

  if (success && !sat_data.empty()) {
    data->first.has_data = false;
    data->second.has_data = false;
    data->third.has_data = false;
    data->fourth.has_data = false;

    auto get_value = [&sat_data](int i) -> uint16_t {
      return sat_data[i] ? *sat_data[i] : 0;
    };

    switch (sat_data.size()) {
      case 16:
        data->fourth.has_data = true;
        data->fourth.satellite_prn_number = get_value(12);
        data->fourth.elevation = static_cast<uint8_t>(get_value(13));
        data->fourth.azimuth = get_value(14);
        data->fourth.snr = static_cast<uint8_t>(get_value(15));
      case 12:
        data->third.has_data = true;
        data->third.satellite_prn_number = get_value(8);
        data->third.elevation = static_cast<uint8_t>(get_value(9));
        data->third.azimuth = get_value(10);
        data->third.snr = static_cast<uint8_t>(get_value(11));
      case 8:
        data->second.has_data = true;
        data->second.satellite_prn_number = get_value(4);
        data->second.elevation = static_cast<uint8_t>(get_value(5));
        data->second.azimuth = get_value(6);
        data->second.snr = static_cast<uint8_t>(get_value(7));
      case 4:
        data->first.has_data = true;
        data->first.satellite_prn_number = get_value(0);
        data->first.elevation = static_cast<uint8_t>(get_value(1));
        data->first.azimuth = get_value(2);
        data->first.snr = static_cast<uint8_t>(get_value(3));
      break;
      default: success = false; break;
    }
  }

  return success;
}


// Checksum calculation
uint8_t nmea::calc_checksum(const std::string& s, bool* ok)
{
  // Calculates CRC of an NMEA sentence (contents between and excluding $ and *)
  if (ok)
    *ok = false;

  // Keeping it GCC 4.9 compatible (can't use rbegin/rend/make_reverse_iterator)
  auto rev_iter = [](auto iter) {
    return std::reverse_iterator<decltype(iter)>(iter);
  };

  // Deduce part of sentence relevant for calculation
  auto first = std::find_if(std::begin(s), std::end(s), [](char c){ return c == '$'; });
  auto last = std::find_if(rev_iter(std::end(s)), rev_iter(std::begin(s)),
                           [](char c){ return c == '*'; }).base();

  // Points to $, step one forward
  if (first != std::end(s))
    std::advance(first, 1);
  // Points to the character after the * due to reversal, step one back
  if (last != std::begin(s))
    std::advance(last, -1);

  if (std::distance(first, last) > 0) {
    if (ok)
      *ok = true;
    return std::accumulate(first, last, static_cast<uint8_t>(0),
        [](uint8_t crc, char c){ return crc ^= static_cast<uint8_t>(c); });
  }

  return 0;
}
