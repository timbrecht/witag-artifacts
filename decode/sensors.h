#ifndef __sensors_h__
#define __sensors_h__

#include <stdio.h>

// ----------------------------------------------------------------------
#define SENSOR_TYPE_INVALID       (-1)
#define SENSOR_TYPE_UNKNOWN       (0)
#define SENSOR_TYPE_TEMP          (1)
#define SENSOR_TYPE_LIGHT         (2)
#define SENSOR_TYPE_ANGLE         (3)

#define SENSOR_VALUE_INVALID  (-2)

// ----------------------------------------------------------------------
int sensor_raw_to_lux(int raw);
int convert_reading(int value, int sensor_type);
void write_value(FILE *outfp[], int value);

#endif

