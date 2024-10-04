#ifndef __expected_h__
#define __expected_h__

#include <stdbool.h>

int expected_str_to_num(char * s);
bool is_expected(int match_index, char *expected);

#endif
