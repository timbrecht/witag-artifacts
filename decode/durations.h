#ifndef __durations_h__
#define __durations_h__

#include "decode.h"

// ----------------------------------------------------------------------
/* 802.11 general constants */
#define DATA_LEN   (302)  // 1560
#define DATA_RATE   (15)
#define DATA_SGI     (1)

extern double t_p[AMPDU_LEN];

// ----------------------------------------------------------------------
void duration_calcs(int preamble_mpdus, int mpdus_per_bit, int bits_per_packet);
double get_PLCP_duration(int rate_idx, int SGI, int length, int aggr_num);

#endif
