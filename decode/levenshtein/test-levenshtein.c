#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "levenshtein.h"

static const bool dbg = true;

// very simple assertion-based demo of a cqueue

int main() {

  char *s1 = "card";
  char *s2 = "cards";
  char *s3 = "carts";
  char *s4 = "cats";
	int result = 0;

	result = levenshtein(s1, strlen(s1), s1, strlen(s1));
	assert(result == 0);
	result = levenshtein(s1, strlen(s1), s2, strlen(s2));
	assert(result == 1);
	result = levenshtein(s2, strlen(s2), s3, strlen(s3));
	assert(result == 1);
	result = levenshtein(s3, strlen(s3), s4, strlen(s4));
	assert(result == 1);
}
