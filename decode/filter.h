#ifndef __filter_h__
#define __filter_h__


#define REG_MATCH            (0)

// ----------------------------------------------------------------------
/* Returns the number of matches */
int is_packet(char *bitmap, regex_t r[], int count, int zero_thold,
       int amble_filter, int line_num, int expected_num, char *packet_str,
       char *packet_regexes[]);

void print_filter_stats(void);
void print_overall_stats(void);

// Used for debugging
void test_preamble_locations(void);

// Print the where we find the first substring that could be preamble
void print_preamble_locations(void);

#endif
