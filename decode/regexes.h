#ifndef __regexes_h__
#define __regexes_h__

void packet_regexes_print(char **regexes);

void packet_regexes_destroy(char **regexes);

char **packet_regexes_create(int bits_per_packet, int mpdus_per_bit, 
       int preamble_mpdus);

void get_preamble_str(char *str, int preamble_mpdus);

void get_postamble_str(char *str, int preamble_mpdus);

#endif
