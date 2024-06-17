/*
Copyright (c) 2024 Traverso Laboratory, Massachusetts Institute of Technology

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef BT_HTS_H_
#define BT_HTS_H_

/**@file
 * @defgroup bt_hts Health Thermometer Service API
 * @{
 * @brief API for the Health Thermometer Service (HTS). 
 * @brief This API implements all optional features of the specification.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/types.h>

// Temperature Type measurement locations. Numbers per 
#define HTS_TEMPTYPE_ARMPIT        1
#define HTS_TEMPTYPE_BODY          2
#define HTS_TEMPTYPE_EAR           3
#define HTS_TEMPTYPE_FINGER        4
#define HTS_TEMPTYPE_GI_TRACT      5
#define HTS_TEMPTYPE_MOUTH         6
#define HTS_TEMPTYPE_RECTUM        7
#define HTS_TEMPTYPE_TOE           8
#define HTS_TEMPTYPE_EAR_DRUM      9

/** Health Thermometer service UUID. */
#define BT_UUID_HEALTH_THERMOMETER_SERVICE 0x1809 

/** HTS characteristics **/
// Temperature Measurement: indicate
#define BT_UUID_HEALTH_THERMOMETER_TEMPERATURE 0x2A1C

// Temperature Type: read
#define BT_UUID_HEALTH_THERMOMETER_TEMPERATURE_TYPE 0x2A1D

// Intermediate Temperature: notify
#define BT_UUID_HEALTH_THERMOMETER_INTERMEDIATE_TEMPERATURE 0x2A1E

// Measurement Interval: read, indicate, write (with authorization)
#define BT_UUID_HEALTH_THERMOMETER_MEASUREMENT_INTERVAL 0x2A21

// characteristic descriptor UUIDs
#define BT_UUID_HEALTH_THERMOMETER_CLIENT_CONFIG 0x2902
#define BT_UUID_HEALTH_THERMOMETER_VALID_RANGE 0x2906


/** HTS Service UUIDs. */
#define HTS_VAL BT_UUID_16_ENCODE(BT_UUID_HEALTH_THERMOMETER_SERVICE)
#define HTS_TEMPERATURE_VAL BT_UUID_16_ENCODE(BT_UUID_HEALTH_THERMOMETER_TEMPERATURE)
#define HTS_TEMPERATURE_TYPE_VAL BT_UUID_16_ENCODE(BT_UUID_HEALTH_THERMOMETER_TEMPERATURE_TYPE)
#define HTS_INTERMEDIATE_TEMPERATURE_VAL BT_UUID_16_ENCODE(BT_UUID_HEALTH_THERMOMETER_INTERMEDIATE_TEMPERATURE)
#define HTS_MEASUREMENT_INTERVAL_VAL BT_UUID_16_ENCODE(BT_UUID_HEALTH_THERMOMETER_MEASUREMENT_INTERVAL)

#define BT_UUID_HTS BT_UUID_DECLARE_16(HTS_VAL)
#define BT_UUID_HTS_TEMPERATURE BT_UUID_DECLARE_16(HTS_TEMPERATURE_VAL)
#define BT_UUID_HTS_TEMPERATURE_TYPE BT_UUID_DECLARE_16(HTS_TEMPERATURE_TYPE_VAL)
#define BT_UUID_HTS_INTERMEDIATE_TEMPERATURE BT_UUID_DECLARE_16(HTS_INTERMEDIATE_TEMPERATURE_VAL)
#define BT_UUID_HTS_MEASUREMENT_INTERVAL BT_UUID_DECLARE_16(HTS_MEASUREMENT_INTERVAL_VAL)

typedef struct int24_t {
    signed int value : 24; //must use unsigned int to avoid implementation-defined behavior
} int24_t;

typedef uint32_t ieee_float32_encoded_t; //for transport over BLE, transmission endianness already handled by platform
typedef struct __attribute__((__packed__)) ieee_float32_decoded_t { //for internal use
  int8_t  exponent; /* Base 10 exponent, 8 bits, signed */
  int24_t mantissa; /* Mantissa, 24 bits, signed, use smallest independent datatype with that capacity */
} ieee_float32_decoded_t;   /* total of 32 bits for payload transmission */

typedef union ieee_float32_t {
    ieee_float32_decoded_t decoded;
    ieee_float32_encoded_t encoded;
} ieee_float32_t;

struct bt_char_properties {
    uint8_t readable : 1;
    uint8_t writeable : 1;
    uint8_t indicateable : 1;
    uint8_t notifyable : 1;
} bt_char_properties;

typedef union HTS_config_t {
        struct {
            uint8_t temperature_type : 4;
            uint8_t intermediate_temperature_enabled : 1;
            uint8_t isFahrenheit : 1;
            uint8_t isTimeOffset : 1;
        } flags;
        uint32_t encoded_bit_flags;
    } hts_config_t;

    typedef struct HTS_Data{
        struct HTS_Config* config;
        ieee_float32_t temperature;
        ieee_float32_t intermediate_temperature;
        ieee_float32_t measurement_interval;
    } HTS_Data;

//special value for NAN temperature measurement as per  IEEE 11073-20601
//used to report an invalid result from a computation step or missing data 
//   due to the hardware’s inability to provide a valid measurement
#define HTS_MEASUREMENT_NAN {.exponent = 0x00, .mantissa = 0x7FFFFF}

//provide a copy of the temperature value sent
typedef void (*hts_temperature_indicated_cb) (struct HTS_Data*); 
//provide a copy of the temperature value sent
typedef void (*hts_intermediate_temperature_notified_cb) (struct HTS_Data*); 

//provide the temperature configuration
typedef void (*hts_temp_config_changed_cb) (struct HTS_Data*); 
//provide the temperature value and the temperature type
typedef void (*hts_temp_type_read_cb) (struct HTS_Data*); 

/** @brief Callback struct used by the LBS Service. */
struct bt_hts_cb {
	hts_temperature_indicated_cb indicated;
    hts_intermediate_temperature_notified_cb intermediate_temperature_notified;
    hts_temp_config_changed_cb temp_config_changed;
    hts_temp_type_read_cb temp_type_read;
};

int bt_hts_init(struct bt_hts_cb *hts_callbacks);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* BT_HTS_H_ */
