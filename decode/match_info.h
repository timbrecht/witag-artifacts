#ifndef __match_info_h__
#define __match_info_h__

#include <stdbool.h>
#include "decode.h"

struct match_info {
   int status;
   bool match[MAX_VALUES];
   bool is_packet;
   int ack_number;
};

#endif
