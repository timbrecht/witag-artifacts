#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// FOR NOW ONLY WORKS WITH 0, 1 or 2 sensors / files.

int num_values = 8;
int outfile_count = 2;

#if 0
int determine_sensor(int value) {

  // If there are multiple sensors figure out which sensor this
  // value/reading belongs to

  // We use the high order bits to mask/determine the sensor.
  // E.g., with 2 sensors the first 0 to 2^(B-1) values are sensor 0
  // and the last 2^(B-1)+1 .. 2^B-1 values are sensor 1

  // For now we just take the 2^B bits and divide it by the number of sensors/files
  // For example 128 values with 3 sensors
  // 0-42 43-84, 85-127

  // 0-41 42-84, 85-127

  // 0-2 => 0
  // 3-5 => 1
  // 6-7 => 2
  
  double increment = num_values / outfile_count;
  int index = 0;
  int tmp = (int) increment;
  printf("value = %d index = %d tmp = %d\n", value, index, tmp);
  while (value > tmp) {
    index++;  
    tmp = (int) (double) index * increment + 1.0;
    printf("value = %d index = %d tmp = %d\n", value, index, tmp);
  }

  return index;
}

int determine_sensor_value(int value, int sensor) {
  double increment = num_values / outfile_count;

  return(value / increment);
}
#endif

int determine_sensor(int value) {

   assert(outfile_count < 3);

   if (value < (num_values / outfile_count)) {
     return 0;
   } else {
     return 1;
   }

}

int determine_sensor_value(int value, int sensor) {
  double increment = num_values / outfile_count;

   if (value < (num_values / outfile_count)) {
     return value;
   } else {
     return(value - (num_values / outfile_count));
   }
}



int main() {
  for (int i=0; i<num_values; i++) {
    printf("%d : sensor = %d sensor_value = %d\n", 
      i, determine_sensor(i), determine_sensor_value(i, determine_sensor(i))); 
  }
}

