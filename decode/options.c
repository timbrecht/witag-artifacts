#include <limits.h>
#include "sensors.h"
#include "decode-algs.h"
#include "options.h"

int bits_per_packet = -1;
int mpdus_per_bit = -1;
int preamble_mpdus = -1;
int expected_value = -1;
int min_zeros = -1;
int gap_thold = INT_MAX;
int rate_gap_thold = INT_MAX;
int filter_and_print_packets = 0;
int quiet_mode = 0;
int safe_mode = 0;

int consecutive_zeros = 0;
int ampdu_greater_than = 0;

double factor = 1;
double offset = 0;

int too_short_len = 0;
int filter_using_estimates = 0;

int algorithm = ALG_DEFAULT;
int expected_in_file = 1;
int exact_lev_len = 0;

int sensor_type = SENSOR_TYPE_UNKNOWN;
int sensor0_type = SENSOR_TYPE_UNKNOWN;
int sensor1_type = SENSOR_TYPE_UNKNOWN;

char outfile1[PATH_MAX] = {0};
char **outfiles = 0;
FILE **outfps = 0;
char infile[PATH_MAX] = {0};
FILE *infp = NULL;
int outfile_count = 0;

// TODO: Think about if this is a good value
int zero_thold = INT_MAX;
int amble_filter = 0;

int delay = 0;
int all_of_file = 0;
int packet_filter = 1;
int num_consecutive = 0;

int min_packet_count = 0;

int verbose = 0;
int expected_num = 0;

int regex_index = DEFAULT_REGEX_INDEX;

int overlap_packet_count = 0;

// TODO: Talk to Kamran about this
int max_mpdus_p_ampdu = DEFAULT_MPDUS_PER_AMPDU;

// Changed to make this a bit more flexible and with fewer checks
int plen_short_by = 1;  // Set some defaults that might not be good
int plen_long_by = 3;

#ifdef OLDWAY
int wait_time = 3000;  // Default (most often used is 3 milliseconds)
#else
int wait_time = 2715;  // New Default 
#endif

int preamble_index = 0;
int extra_ones_added = 1;
int print_bit_error_rate = 0;
