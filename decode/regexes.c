#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "regexes.h"

// ----------------------------------------------------------------------
static const bool dbg = false;

#define MAX_STR_LEN   (128)
#define NUM_STRINGS     (1)

#ifdef OLDWAY_FIVE_BITS
  /* Bit counts are not including the first and last 1's */
  /* 1... ... ... ... ...1 */
  /* 0123 456 789 012 3456 */
  /* 15 bits would be the ideal situation but we don't always get that */
  /* Middle/data bits could be 8,9 or 10 */
  /* 14 bits  */ "10{3}[0-1]{8}0{3}1",
  /* 14 bits  */ "10{3}[0-1]{9}0{2}1",
  /* 14 bits  */ "10{2}[0-1]{9}0{3}1",
  /* 14 bits  */ "10{2}[0-1]{10}0{2}1",
  // 
  /* 15 bits  */ "10{4}[0-1]{8}0{3}1",
  /* 15 bits  */ "10{3}[0-1]{8}0{4}1",
  /* 15 bits  */ "10{3}[0-1]{9}0{3}1",
  /* 15 bits  */ "10{2}[0-1]{9}0{4}1",
  /* 15 bits  */ "10{4}[0-1]{9}0{2}1",
  /* 15 bits  */ "10{2}[0-1]{10}0{3}1",
  /* 15 bits  */ "10{3}[0-1]{10}0{2}1",
  // 
  /* 16 bits  */ "10{4}[0-1]{8}0{4}1",
  /* 16 bits  */ "10{3}[0-1]{9}0{4}1",
  /* 16 bits  */ "10{4}[0-1]{9}0{3}1",
  /* 16 bits  */ "10{3}[0-1]{10}0{3}1",
  /* 16 bits  */ "10{2}[0-1]{10}0{4}1",
  /* 16 bits  */ "10{4}[0-1]{10}0{2}1",
  /* 16 bits  */ NULL
#endif

// ----------------------------------------------------------------------
void packet_regexes_print(char **regexes) 
{
   int index = 0;
   char *cur = NULL;
	 printf("Using the following regexecs to match packets\n");
   while ((cur = regexes[index]) != NULL) {
     printf("regexe[%d] = %s\n", index, cur);
     index++;
   }
}

// ----------------------------------------------------------------------
void packet_regexes_destroy(char **regexes)
{
   assert(regexes);
   int index = 0;
   while (regexes[index] != NULL) {
     free(regexes[index]);
     regexes[index] = NULL;
     index++;
   }
   free(regexes);
   regexes = NULL;
}
  
// ----------------------------------------------------------------------
char **packet_regexes_create(int bits_per_packet, int mpdus_per_bit, 
       int preamble_mpdus)
{
	assert(bits_per_packet >= 0);
	assert(mpdus_per_bit >= 0);
	assert(preamble_mpdus >= 0);
  assert(mpdus_per_bit + preamble_mpdus > 0);

  if (dbg) printf("b = %d m = %d p = %d\n", 
           bits_per_packet, mpdus_per_bit, preamble_mpdus);

  // The last one will be a null pointer (sentinal value).
	char **strings = malloc((NUM_STRINGS + 1) * sizeof(char *));
	assert(strings);

  
  // Don't need to allocate memory for the last entry
  // We'll set that one to null later below
	for (int i = 0; i < NUM_STRINGS; i++) {
	  strings[i] = malloc((MAX_STR_LEN+1) * sizeof(char));
		char *str = strings[i];
		assert(str);
    for (int j = 0; j<(MAX_STR_LEN+1); j++) {
      str[j] = '\0';
    }

		// Example for 5 bits, 3 mpdus per bit and preamble of 3
		// "10{2,4}[0-1]{14,16}0{2}1",

    char tmp[MAX_STR_LEN+1] = {'\0'};

		// Create the premable
		sprintf(tmp, "10{%d,%d}", preamble_mpdus-1, preamble_mpdus+1);
		strcat(str, tmp);

		// Now add the bits
#ifdef OLDWAY
		sprintf(tmp, "[0-1]{%d,%d}", (bits_per_packet * mpdus_per_bit) - 1,
									(bits_per_packet * mpdus_per_bit) + 1);
#else
    // TODO: Need to look at this carefully and figure out:
    //       How to do this generally?
    //       Do we need different versions for different devices?
    //       Look at how this interacts with Levenstein
		sprintf(tmp, "[0-1]{%d,%d}", (bits_per_packet * mpdus_per_bit) - 1,
									(bits_per_packet * (mpdus_per_bit+1) + 2));
#endif
		strcat(str, tmp);

		// Now add the postamble
		sprintf(tmp, "0{%d,%d}1", preamble_mpdus - 1, preamble_mpdus + 1);
		strcat(str, tmp);
	}

	strings[NUM_STRINGS] = NULL;

  // if (dbg) packet_regexes_print(packet_regex_options[bits_per_packet]);
  // return packet_regex_options[bits_per_packet];

  packet_regexes_print(strings);
  return strings;
}

// ----------------------------------------------------------------------
void get_preamble_str(char *str, int preamble_mpdus) 
{
  strcat(str, "1");
  for (int i=0; i<preamble_mpdus - 1; i++) {
    strcat(str, "0");
  }
}

// ----------------------------------------------------------------------
void get_postamble_str(char *str, int preamble_mpdus) 
{
  for (int i=0; i<preamble_mpdus - 1; i++) {
    strcat(str, "0");
  }
  strcat(str, "1");
}
