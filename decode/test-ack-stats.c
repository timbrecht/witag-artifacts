#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ack-stats.h"

// ----------------------------------------------------------------------
int main(int argc, char *argv[]) 
{
#if 0
  if (argc != 4) {
    printf("Usage: %s bits_per_packet mpdus_per_bit preamble_mpdus\n", argv[0]);
    exit(1);
  }
  int b = atoi(argv[1]);
  int m = atoi(argv[2]);
  int p = atoi(argv[3]);
#endif
  
	int window = 5;
	int max_bits = 64;
  struct ack_stats *a = ack_stats_create(max_bits, window);
	assert(a);
  ack_stats_update(a, 10, 5);
	ack_stats_print(a);
  ack_stats_update(a, 10, 5);
	ack_stats_print(a);
  ack_stats_update(a, 10, 5);
	ack_stats_print(a);
  ack_stats_update(a, 10, 5);
	ack_stats_print(a);
  ack_stats_update(a, 10, 5);
	ack_stats_print(a);

  ack_stats_update(a, 64, 64);
	ack_stats_print(a);

	ack_stats_lengths_print(a);
}
