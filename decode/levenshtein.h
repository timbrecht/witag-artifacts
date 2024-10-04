#ifndef __levenshtein_h__
#define __levenshtein_h__


// Generate all strings with 
//    b bits where each bit has 
//    m mpdus per bit and we use
//    p bits for pre and postamble
//    count is set to tell us how many strings there are
char **create_lev_strings(int b, int m, int p, int *count);

// Destroy the strings (frees them)
void destroy_lev_strings(char *s[], int count);

// Generate all 2^bits strings
char **create_expected_strs(int bits);

void destroy_expected_strs(char **strings, int count);

// return levenshtein (edit distance) between s and t
int levenshtein(const char *s, const char *t);

#endif
