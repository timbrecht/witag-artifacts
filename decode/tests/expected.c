
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "utils.h"

#define MPDUS_PER_BIT (3)
#define AMBLE_ZEROS   (3)

void print_preamble() 
{
  printf("1");
  for (int i=0; i<AMBLE_ZEROS; i++) {
    printf("0");
	}
}

void print_postamble() 
{
  for (int i=0; i<AMBLE_ZEROS; i++) {
    printf("0");
	}
  printf("1");
}

void print_in_binary(int num, int max_bits) 
{
	int c = 0;
	int tmp = 0;

  for (c = 31; c >= 0; c--) {
    tmp = num >> c;

    if (c <= max_bits+1) {
			if (tmp & 1) {
			  for (int j=0; j<MPDUS_PER_BIT; j++) {
				  printf("1");
				}
			} else {
			  for (int j=0; j<MPDUS_PER_BIT; j++) {
				  printf("0");
				}
			}
		}
  }
}


int main(int argc, char *argv[])
{
   if (argc != 2) {
	   printf("Usage: %s <bits>\n", argv[0]);
		 exit(1);
	 }

   int bits = atoi(argv[1]);
	 printf("Bits = %d\n", bits);
	 char binary[100] = {0};

	 assert(mypow(2, 0) == 1);
	 assert(mypow(2, 1) == 2);
	 assert(mypow(2, 2) == 4);
	 assert(mypow(2, 3) == 8);

	 int max = mypow(2, bits);

	 for (int i=0; i<max; i++) {
	   printf(" /* %4d */ ", i);
		 printf("\"");
		 print_preamble();
		 print_in_binary(i, MPDUS_PER_BIT);
		 print_postamble();
		 printf("\",");
     printf("\n");
	 }
}

