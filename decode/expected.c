#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "globals.h"
#include "expected.h"

// ----------------------------------------------------------------------
int expected_str_to_num(char * s) {
  // printf("expected_str_to_num %s\n", s);
  for (int i=0; i<num_values; i++) {
    if (strcmp(s, expected_str[i]) == 0) {
      return i;
    }
  }
  assert(1 == 0);
}

// ----------------------------------------------------------------------
/* Is the index (number found) equal to the expected value */
bool is_expected(int match_index, char *expected)
{
  assert(match_index >= 0);
  assert(match_index < num_values);
  // printf("match_index = %d expected_str = %s expected = %s\n",
  //        match_index, expected_str[match_index], expected);
  if (strcmp(expected_str[match_index], expected) == 0) {
    return true;
  }
  return false;
}
