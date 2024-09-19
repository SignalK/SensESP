#ifndef SENSESP_TYPES_POSITION_H_
#define SENSESP_TYPES_POSITION_H_

#include <ArduinoJson.h>
#include <limits>

namespace sensesp {

/// Value used to indicate an invalid or missing altitude
constexpr double kInvalidDouble = std::numeric_limits<double>::lowest();
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

  Position() : latitude(kInvalidDouble), longitude(kInvalidDouble) {}
  Position(double latitude, double longitude,
           float altitude = kPositionInvalidAltitude)
      : latitude(latitude), longitude(longitude), altitude(altitude) {}
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

  ENUVector() : east(kInvalidDouble), north(kInvalidDouble) {}
  ENUVector(float east, float north, float up = kPositionInvalidAltitude)
      : east(east), north(north), up(up) {}
};

/**
 * @brief Container for attitude data.
 *
 * Roll, pitch, and yaw (heading) angles, in radians.
 *
 */
struct AttitudeVector {
  // Ordering chosen to match the order of fields in Signal K
  float roll;
  float pitch;
  float yaw;  // heading

  AttitudeVector()
      : roll(kInvalidDouble), pitch(kInvalidDouble), yaw(kInvalidDouble) {}
  AttitudeVector(float roll, float pitch, float yaw)
      : roll(roll), pitch(pitch), yaw(yaw) {}
};

/**
 * @brief Adds support in ArduinoJson to deserialize Position type data.
 *
 * This is an ArduinoJson custom converter to convert position type data from
 * JSON into a Position struct. The JSON data must at least contain "latitude"
 * and "longitude", "altitude" is optional. ArduinoJson will automatically
 * call this function whenever it has to convert a JSON to a Position struct.
 * More info here: https://arduinojson.org/news/2021/05/04/version-6-18-0/
 *
 * @param src The JSON document containing the position data
 * @param dst The address to a Position struct to write the data to
 */
void convertFromJson(JsonVariantConst src, Position &dst);
void convertFromJson(JsonVariantConst src, ENUVector &dst);
void convertFromJson(JsonVariantConst src, AttitudeVector &dst);

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
bool canConvertFromJson(JsonVariantConst src, const ENUVector &);
bool canConvertFromJson(JsonVariantConst src, const AttitudeVector &);

void convertToJson(const Position &src, JsonVariant dst);
void convertToJson(const ENUVector &src, JsonVariant dst);
void convertToJson(const AttitudeVector &src, JsonVariant dst);

}  // namespace sensesp

#endif  // SENSESP_TYPES_POSITION_H_
