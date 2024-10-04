#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "sensors.h"
#include "options.h"
#include "globals.h"
// ----------------------------------------------------------------------

#define DIFFERENT_VALUES  (32)
#define ADJUST_BY         (10)

static int mapping[DIFFERENT_VALUES] = {
  /*  0 */   0,
  /*  1 */   0,
  /*  2 */   0,
  /*  3 */   0,
  /*  4 */   0,
  /*  5 */   0,
  /*  6 */   0,
  /*  7 */  15,
  /*  8 */  18,
  /*  9 */  20,
  /* 10 */  24,
  /* 11 */  27,
  /* 12 */  30,
  /* 13 */  46,
  /* 14 */  58,
  /* 15 */  61,
  /* 16 */  74,
  /* 17 */  98,
  /* 18 */ 106,
  /* 19 */ 127,
  /* 20 */ 138,
  /* 21 */ 156,
  /* 22 */ 232,
  /* 23 */ 346,
  /* 24 */ 437,
  /* 25 */ 551,
  /* 26 */ 613,
  /* 27 */ 792,
  /* 28 */ 875,
  /* 29 */ 935,
  /* 30 */ 1138,
  /* 31 */ 1207,
};

static int count[DIFFERENT_VALUES] = {0};

// conversion for light sensor with 10K resistor.

// ----------------------------------------------------------------------
// #define VIN 5 // V power voltage
// #define R 10000 
int sensor_raw_to_lux(int raw)
{
#ifdef OLDWAY
  // Conversion rule
        const double volts_in = 5.0;
        const double r = 10000.0;

        // Conversion analog to voltage
  double volts_out = (double) raw * (volts_in / 1024.0);
  
        // Conversion voltage to resistance for light dependent resistor
  double rldr = (r * (volts_in - volts_out)) / volts_out; 
  double t = (rldr / 1000.0);
        // Conversion resitance to lumen
  double lumen = 500.0 / t; 
  return (int) lumen;
#else
   int value = 0;
   int adjust = 0;

   // TODO: this is a hack to try to get something closer
   //       to LUX / Lumens because the above wasn't working very well.
   value = mapping[raw];
   
   // used to randomize things a little  
   count[raw]++;

   if (count[raw] % 247 == 0) {
     adjust = count[raw] % ADJUST_BY;
     // Sometimes decrease and sometimes add
     if (count[raw] % 2 == 0) {
       value = value - adjust;
     } else {
       value = value + adjust;
     }

     if (value < 0 || raw < 7) {
       value = 0;
     }

     mapping[raw] = value;

   }

   // printf("raw = %d count = %d value = %d\n", raw, count[raw], value);
   return value;
#endif
}


// ----------------------------------------------------------------------
int convert_reading(int value, int sensor_type) 
{
  double new_value = SENSOR_VALUE_INVALID;
  switch (sensor_type) {
    case SENSOR_TYPE_UNKNOWN:
    case SENSOR_TYPE_ANGLE:
      new_value = (value * factor) + offset;
      break;

    case SENSOR_TYPE_TEMP:
      new_value = (value * factor) + offset;
      break;

    case SENSOR_TYPE_LIGHT:
      new_value = (value * factor) + offset;
      new_value = sensor_raw_to_lux(new_value);
      break;

    default:
      printf("Sensor type = %d is invalid\n", sensor_type);
      exit(1);
      break;
  }
  return((int) new_value);
}

// ----------------------------------------------------------------------
int determine_sensor(int value) {

   assert(outfile_count < 3);

   if (value < (num_values / outfile_count)) {
     return 0;
   } else {
     return 1;
   }

}

int determine_sensor_value(int value, int sensor) {

   if (value < (num_values / outfile_count)) {
     return value;
   } else {
     return(value - (num_values / outfile_count));
   }
}


// ----------------------------------------------------------------------
void write_value(FILE *outfps[], int value)
{
  assert(outfps);

  int sensor_index = determine_sensor(value);
  int sensor_value = determine_sensor_value(value, sensor_index);

  int to_write = SENSOR_VALUE_INVALID;

  // If for some reason we aren't getting any packets the value
  // won't be known so don't write it, just leave the previous value
  if (safe_mode) {
    if (sensor_value == SENSOR_VALUE_INVALID) {
      return;
    }
  }

#ifdef DONE_ELESWHERE_NOW
  if (outfp == NULL) {
    outfp = fopen(outfile, "w");
    if (outfp == NULL) {
       fprintf(stderr, "Error: Unable to open output file %s\n", outfile);
       printf("Error: Unable to open output file %s\n", outfile);
       exit(1);
    }
  }
#endif

  int sensor_type = SENSOR_TYPE_INVALID; 
  switch (sensor_index) {
    case 0:
      sensor_type = sensor0_type;
      break;

    case 1:
      sensor_type = sensor1_type;
      break;

    default:
      printf("Invalid sensor index - currently only support 2 sensor types.\n");
      printf("One for sensor 0 and one for sensor 1\n");
      exit(1);
  }

  to_write = convert_reading(sensor_value, sensor_type);

  fseek(outfp, 0L, SEEK_SET);
  fprintf(outfp, "%8d\n", to_write);
}
