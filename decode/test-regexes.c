#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "regexes.h"

int main(int argc, char *argv[]) 
{
  if (argc != 4) {
    printf("Usage: %s bits_per_packet mpdus_per_bit preamble_mpdus\n", argv[0]);
    exit(1);
  }
  int b = atoi(argv[1]);
  int m = atoi(argv[2]);
  int p = atoi(argv[3]);
  
  char **use_regexes = packet_regexes_create(b, m, p);
  packet_regexes_print(use_regexes);
}
