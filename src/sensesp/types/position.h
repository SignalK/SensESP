#ifndef SENSESP_TYPES_POSITION_H_
#define SENSESP_TYPES_POSITION_H_

#include <ArduinoJson.h>
#include <limits>

namespace sensesp {

/// Value used to indicate an invalid or missing altitude
constexpr float kPositionInvalidAltitude = std::numeric_limits<float>::lowest();

/**
 * @brief Position data container.
 *
 * Position data as latitudes and longitudes, in decimal degrees, and altitude,
 * in meters.
 *
 */
struct Position {
  double latitude;
  double longitude;
  float altitude = kPositionInvalidAltitude;
};

/**
 * @brief Container for local tangent plane coordinates.
 *
 * East-North-Up coordinates, in reference to the local tangent plane.
 * Static locations are in meters, velocities in m/s.
 *
 */
struct ENUVector {
  float east;
  float north;
  float up = kPositionInvalidAltitude;
};

/**
 * @brief Adds support in ArduinoJson to deserialize Position type data.
 *
 * This is an ArduinoJson custom converter to convert position type data from
 * JSON into a Postion struct. The JSON data must at least contain "latitude"
 * and "longitude", "altitude" is optional. ArduinoJson will automatically
 * call this function whenever it has to convert a JSON to a Position struct.
 * More info here: https://arduinojson.org/news/2021/05/04/version-6-18-0/
 *
 * @param src The JSON document containing the position data
 * @param dst The address to a Position struct to write the data to
 */
void convertFromJson(JsonVariantConst src, Position &dst);

/**
 * @brief Tells ArduinoJson whether the given JSON is a Position or not
 *
 * This function is automatically called by ArduinoJson whenever it has to
 * convert a JSON to a Position struct.
 *
 * @param src A JSON document
 * @return true if the given JSON contains "latitude" and "longitude" keys
 * @return false if the given JSON doesn't contain "latitude" or "longitude"
 * keys
 */
bool canConvertFromJson(JsonVariantConst src, const Position &);

}  // namespace sensesp

#endif  // SENSESP_TYPES_POSITION_H_
