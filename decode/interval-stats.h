#ifndef __interval_stats_h__
#define __interval_stats_h__

#include "decode.h"

extern double Exp_tag_pkt;
extern int dt_packet_count;
extern int dt_per_packet_count;
extern int dt_blockack_count;
extern double rate_gap;
extern int  cumm_t_f[AMPDU_LEN];
extern int cumm_per_packet_count;
extern int cumm_packet_count;
extern int cumm_blockack_count;
extern double cumm_rate_gap;
extern int cumm_ampdu_len;
extern int cumm_len_one;

extern int delta_ampdu_len;
extern int delta_len_one;

extern int t_f[AMPDU_LEN];
extern double t_p[AMPDU_LEN];
extern double t_packet_rate;

#endif
