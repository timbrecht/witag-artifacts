#ifndef __decode_h__
#define __decode_h__

// Maximum AMPDU_LEN we are expecting to handle
#define AMPDU_LEN          (64)
#define MAX_BITS    (AMPDU_LEN)

// NOTE that several arrays are sizes using this so
//   don't make it large unless you are using a larger b
// TODO: Make this size dynamic according to b (bits per packet)
// What is the maximum number of different values supported
// (i.e., 2^bits)
// #define MAX_VALUES         (32)  // For the demo
// #define MAX_VALUES         (256)
#define MAX_VALUES         (1024)

// If we don't know the value of the data in the packet/message
#define VALUE_INVALID       (-1)

// ----------------------------------------------------------------------
extern int expected_packet_len(void);

#endif
