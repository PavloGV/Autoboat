#ifndef NMEA2000_H
#define NMEA2000_H

/**************************************************************************************************
 * @file
 * This library implements some helper functions and decoders for some messages in the NMEA2000 
 * protocol. This protocol is little-Endian with details on messages found at:
 * http://www.keversoft.com/Keversoft/Packetlogger.html
 *
 * Testing this library:
 *
 * Have in the same directory the external requirements: types.h
 *
 * On a system with GCC run: gcc -Wall nmea2000.c -DUNIT_TEST_NMEA2000 -lm
 *
 * Location of floorf, fmodf --> in the math library so you need to link against
 * it by adding '-lm'
 *
 * '-DUNIT_TEST_NMEA2000' --> Specifies a UNIT_TEST_NMEA2000 constant which enabled
 * the main() at the end for use in unit testing this library
 *
 *************************************************************************************************/

#include <stdint.h>

/**
 * Helper functions.
 */

/**
  * Converts the 29-bit CAN extended address into its components according to the ISO 11783 spec.
  * @param[in] can_id The 29-bit CAN identifier to decode.
  * @param[out] src The source of this message.
  * @param[out] dest The destination for this message. Set to 255 if the message was a broadcast.
  * @param[out] pri The priority, a 3-bit number with higher values indicating higher priority.
  * @return The PGN of for this ID.
  */
uint32_t Iso11783Decode(uint32_t can_id, uint8_t *src, uint8_t *dest, uint8_t *pri);

/**
  * Encodes a 29-bit CAN header given the necessary parameters. Note that there are two
  * separate styles of encoding: PDU1 is a one-to-one communication with a destination,
  * PDU2 is a broadcast without a destination. As the destination and PGN can both encode
  * this information and given the expected usage of this function, the PGN is given
  * priority for determining this.
  * @param[in] pgn The PGN number used for this message.
  * @param[in] src The source of this message.
  * @param[in] dest The destination for this message. Set to 255 if the message was a broadcast.
  * @param[in] pri The priority, a 3-bit number with higher values indicating higher priority.
  * @return The encoded 29-bit CAN identifier (as a uint32)
  */
uint32_t Iso11783Encode(uint32_t pgn, uint8_t src, uint8_t dest, uint8_t pri);

// Given a number of days return date. Accounts properly for leap days.
void DaysSinceEpochToYMD(uint16_t days, uint16_t *year, uint16_t *month, uint16_t *day);

/**
 * Calculate the current Unix timestamp in microseconds. Note that no validation is done on the
 * input parameters.
 *
 * @param usecsFromMidnight The number of microseconds since midnight.
 * @param daysSinceEpoch The number of days since epoch.
 * @return The number of microseconds since epoch.
 */
uint64_t UsecondsSinceEpoch(uint64_t usecsFromMidnight, uint16_t daysSinceEpoch);

/***
 * PGN Parsers
 */

 // Units are year: absolute, month: 1-12, day: 1-31, hour: 0-23, min: 0-59, sec: 0-59, seqId: none, source: 0 (GPS), 1 (GLONASS), 2 (Radio station), 3 (Local cesium clock), 4 (local rubidium clock), 5 (Local crystal clock).
 // NOTE: The usecSinceEpoch value is not part of the return value bitfield and is only valid if the time given within the message was valid.
uint8_t ParsePgn126992(const uint8_t data[8], uint8_t *seqId, uint8_t *source, uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *hour, uint8_t *minute, uint8_t *second, uint64_t *usecSinceEpoch);

// Units are instance: none, direction: none/enum, angleOrder: .0001 Radians, position: .0001 Radians.
uint8_t ParsePgn127245(const uint8_t data[6], uint8_t *instance, uint8_t *direction, float *angleOrder, float *position);

// Units are seqId: none, varSource: enum, ageInDays: days since epoch, variation: radians with positive Easterly.
uint8_t ParsePgn127258(const uint8_t data[8], uint8_t *seqId, uint8_t *varSource, uint16_t *ageOfService, float *variation);

/**
  * Decodes PGN 127508 - Battery Status
  * @param[in] data The 8 payload bytes of the CAN message.
  * @param[out] instance A unique ID for this battery instance.
  * @param[out] voltage The voltage in Volts of this battery. If invalid is NaN.
  * @param[out] current The current in Amperes being drawn from this battery. If invalid is NaN.
  * @param[out] temperature The current temperature in degrees Celsius of the battery. If invalid is NaN.
  * @param[out] seqId The sequence ID for this data. Other messages from the same source sharing this sequence ID involves data measurements from the same timestep.
  * @return A bitfield containing success (1) or failure (0) for data from each output argument. For success to be indicated the corresponding argument must be non-null and have valid data within the message.
  */
uint8_t ParsePgn127508(const uint8_t data[8], uint8_t *seqId, uint8_t *instance, float *voltage, float *current, float *temperature);

// Units are seqId: none, waterSpeed: m/s.
uint8_t ParsePgn128259(const uint8_t data[8], uint8_t *seqId, float *waterSpeed);

// Units are seqId: none, waterDepth: m, offset: m.
uint8_t ParsePgn128267(const uint8_t data[7], uint8_t *seqId, float *waterDepth, float *offset);

// Units are seqId: none, latitude: radians (+ north), longitude: radians (+ east).
// NOTE: Latitude and longitude are both returned as int32s because there is loss of precision if
// real32 datatypes are used.
uint8_t ParsePgn129025(const uint8_t data[8], int32_t *latitude, int32_t *longitude);

// Units are seqId: none, cogRef: 0 (True), 1 (magnetic), cog: radians eastward from north, sog: m/s
uint8_t ParsePgn129026(const uint8_t data[8], uint8_t *seqId, uint8_t *cogRef, uint16_t *cog, uint16_t *sog);

// Units are seqId: none, desiredMode: enum PGN_129539_MODE, actualMode: PGN_129539_MODE, hdop: .01 unitless, vdop: .01 unitless, tdop: .01 unitless
uint8_t ParsePgn129539(const uint8_t data[8], uint8_t *seqId, uint8_t *desiredMode, uint8_t *actualMode, uint16_t *hdop, uint16_t *vdop, uint16_t *tdop);

// Units are seqId: none, airSpeed: m/s, direction: radians eastward from north.
uint8_t ParsePgn130306(const uint8_t data[8], uint8_t *seqId, float *airSpeed, float *direction);

// Units are seqId: none, waterTemp: degrees C, airTemp: degrees C, and airPressure: kPa.
uint8_t ParsePgn130310(const uint8_t data[8], uint8_t *seqId, float *waterTemp, float *airTemp, float *airPressure);

// Units are seqId: none, tempInstance: none/enum, humidityInstance: none/enum, temp: degrees C, humidity: %, pressure: kPa.
uint8_t ParsePgn130311(const uint8_t data[8], uint8_t *seqId, uint8_t *tempInstance, uint8_t *humidityInstance, float *temp, float *humidity, float *pressure);

/**
 * Define nice constants for dealing with all the PGN numbers.
 */
enum PGN {
    PGN_SYSTEM_TIME      = 126992,
    PGN_RUDDER           = 127245,
    PGN_MAG_VARIATION    = 127258,
    PGN_BATTERY_STATUS   = 127508,
    PGN_SPEED            = 128259,
    PGN_WATER_DEPTH      = 128267,
    PGN_POSITION_RAP_UPD = 129025,
    PGN_COG_SOG_RAP_UPD  = 129026,
    PGN_TIME_DATE        = 129033,
    PGN_GNSS_DOPS        = 129539,
    PGN_WIND_DATA        = 130306,
    PGN_ENV_PARAMETERS   = 130310,
    PGN_ENV_PARAMETERS2  = 130311
};

/**
 * Define constants for use with the SID (sequence ID).
 */
enum PGN_SID {
    PGN_SID_INVALID = 0xFF
};

/**
 * Defines the different possible values for the VARIATION_SOURCE field in PGN127258.
 */
enum PGN127258_VARIATION_SOURCE {
	PGN127258_VARIATION_SOURCE_MANUAL     = 0,
	PGN127258_VARIATION_SOURCE_AUTO_CHART = 1,
	PGN127258_VARIATION_SOURCE_AUTO_TABLE = 2,
	PGN127258_VARIATION_SOURCE_AUTO_CALC  = 3,
	PGN127258_VARIATION_SOURCE_WMM2000    = 4,
	PGN127258_VARIATION_SOURCE_WMM2005    = 5,
	PGN127258_VARIATION_SOURCE_WMM2010    = 6,
	PGN127258_VARIATION_SOURCE_WMM2015    = 7,
	PGN127258_VARIATION_SOURCE_WMM2020    = 8
};

/**
 * Define constants for PGN129539 indicating the GPS unit's operating mode.
 */
enum PGN_129539_MODE {
	PGN_129539_MODE_1D    = 0,
	PGN_129539_MODE_2D    = 1,
	PGN_129539_MODE_3D    = 2,
	PGN_129539_MODE_AUTO  = 3,
	PGN_129539_MODE_RES1  = 4,
	PGN_129539_MODE_RES2  = 5,
	PGN_129539_MODE_ERROR = 6,
	PGN_129539_MODE_INV   = 7
};

/**
 * Define constants for use with the temperature instance field of PGN 130311.
 */
enum PGN_130311_TEMP_INST {
    PGN_130311_TEMP_INST_SEA         = 0,
    PGN_130311_TEMP_INST_OUTSIDE     = 1,
    PGN_130311_TEMP_INST_INSIDE      = 2,
    PGN_130311_TEMP_INST_ENGINE_ROOM = 3,
    PGN_130311_TEMP_INST_MAIN_CABIN  = 4,
    PGN_130311_TEMP_INST_INVALID     = 0x3F
};

/**
 * Define constants for use with the humidity instance field of PGN 130311.
 */
enum PGN_130311_HUMID_INST {
    PGN_130311_HUMID_INST_INSIDE  = 0,
    PGN_130311_HUMID_INST_OUTSIDE = 1,
    PGN_130311_HUMID_INST_INVALID = 3,
};

#endif // NMEA2000_H
