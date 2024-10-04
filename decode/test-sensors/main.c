#include <stdio.h>
#include "../sensors.h"

int factor = 31;
int offset = 0;

int
main() {
  printf("%2s : %5s -> %4s\n", "i", "Value", "LUX");
  for (int i=0; i<32; i++) {
	  int value = (i * factor) + offset;
    printf("%2d : %5d -> %4d\n", i, value, sensor_raw_to_lux(value));
  }
}
