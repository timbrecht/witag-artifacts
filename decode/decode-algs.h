#ifndef __decode_algs_h__
#define __decode_algs_h__

#include <regex.h>

#define ALG_REGEX            (0)
#define ALG_LEV              (1)
#define ALG_DEFAULT          (ALG_LEV)

#define DEFAULT_REGEX_INDEX  (2)
#define NUM_REGEXES          (6)

// ----------------------------------------------------------------------
extern char **value_regex_options[NUM_REGEXES];
extern char **value_regexes;

void detect_with_lev(char *packet, int line_count,
     int expected_num, char *expected, int verbose,  double delta_sec, char *bitmap);
void detect_with_regex(char *packet, regex_t num_regex[],
    int line_count, int expected_num, char *expected, int verbose);

#endif
