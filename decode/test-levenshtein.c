#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <limits.h>
#include "levenshtein.h"

// static const bool dbg = false;

#define NUM_VALUES   (8)

char *matches[NUM_VALUES] = {
  "10000000000000001",
  "10000000001110001",
  "10000001110000001",
  "10000001111110001",
  "10001110000000001",
  "10001110001110001",
  "10001111110000001",
  "10001111111110001",
};

struct check_info {
  char *str;
	int expected;
};


struct check_info check[] = {
  {"10000000111110001",  3},
  {"1000011111000001",   6},
  {"10000110000110001",  5},
  {"10000000110000001",  2},
  {0, 0},
};


int main(int argc, char *argv[]) 
{
  char *s1 = "card";
  char *s2 = "cards";
  char *s3 = "carts";
  char *s4 = "cats";
	int result = 0;
	char *src = 0;

	result = levenshtein(s1, s1);
	assert(result == 0);
	result = levenshtein(s1, s2);
	assert(result == 1);
	result = levenshtein(s2, s3);
	assert(result == 1);
	result = levenshtein(s3, s4);
	assert(result == 1);

  int index = 0;
  while ((src = check[index].str)) {
	  int min = INT_MAX;
	  int min_index = -1;
    for (int i=0; i<NUM_VALUES; i++) {
	    result = levenshtein(src, matches[i]);
		  printf("src = [%s] distance from %d = %d\n", src, i, result);
			if (result < min) {
        min = result;
				min_index = i;
			}
	  }
		printf("src = [%s] prediction min distance = %d value = %d, actual = %d\n",
		        src, min, min_index, check[index].expected);
		printf("-----------------------------------\n");
		index++;
	}

  // b, m, p, *count
	int count = 0;
  char **lev_strings = create_lev_strings(5, 3, 3, &count);
	printf("count = %d\n", count);
	for (int i = 0; i < count; i++) {
    printf("lev_strings[%3d] = %s\n", i, lev_strings[i]);
	}

	destroy_lev_strings(lev_strings, count);

  // b, m, p, *count
	count = 0;
  lev_strings = create_lev_strings(6, 4, 3, &count);
	printf("count = %d\n", count);
	for (int i = 0; i < count; i++) {
    printf("lev_strings[%3d] = %s\n", i, lev_strings[i]);
	}

  // b, m, p, *count
	count = 0;
  lev_strings = create_lev_strings(5, 8, 8, &count);
	printf("count = %d\n", count);
	for (int i = 0; i < count; i++) {
    printf("lev_strings[%3d] = %s\n", i, lev_strings[i]);
	}

	destroy_lev_strings(lev_strings, count);

}
