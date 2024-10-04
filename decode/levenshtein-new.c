#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils.h"
#include "decode.h"
// ----------------------------------------------------------------------
// NOTE the code for both of these were found online.

#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))
// This is in decode
// #define MAX_BITS (64) // At most 64-bits

static bool const print_lev_strings = false;
static bool const dbg = false;
static bool const dbg_expected = false;
static bool const dbg_malloc = false;
static int total_len = 0;

int levenshtein_orig(char *s1, char *s2);
int levenshtein_new(char *s1, char *s2); 

int levenshtein(char *s1, char *s2) {
  return levenshtein_orig(s1, s2);
}

// ----------------------------------------------------------------------
void reverse(int a[]) 
{
  int front = 0;
	int back = 0;
	int i = 0;
	while (a[i] != -1) {
	  i++;
	}
	back = i-1;

	while (front < back) {
	  int tmp = a[front];
	  a[front] = a[back];
		a[back] = tmp;
		front++;
		back--;
	}
}

// ----------------------------------------------------------------------
void to_binary(int n, int a[]) 
{
  int num = n;
	int i = 0;
	if (dbg) printf("to_binary: num = %d\n", num);

	if (n == 0) {
    a[0] = 0;
		a[1] = -1;
	  printf("to_binary: num = %d => ", num);
		printf("%d", a[0]);
		printf("\n");
		return;
	}

  for (i = 0; n > 0; i++) {    
    a[i] = n % 2;    
	  n = n / 2;    
		assert(a[i] == 0 || a[i] == 1);
	  if (dbg) printf("to_binary: a[%d] = %d\n", i, a[i]);
  }    
	a[i] = -1;

	reverse(a);
	if (dbg) printf("to_binary: a[%d] = %d\n", i, a[i]);

	if (dbg) {
		i = 0;
	  printf("to_binary: num = %d => ", num);
	  while (a[i] != -1) {
		  printf("%d", a[i]);
			i++;
		}
		printf("\n");
	}

}

// ----------------------------------------------------------------------
void add_num_bits(char *s, int n, int value) 
{
  if (dbg_malloc) printf("s = [%s] len = %d n = %d value = %d total_len+1 = %d\n",
                  s, (int) strlen(s), n, value, total_len+1);
  if (dbg_malloc) assert((strlen(s) + n + 1) <= total_len+1);
  assert(value == 0 || value == 1);

  int location = strlen(s);
  for (int i = 0; i < n; i++) {
	  s[location] = value + '0';
		location++;
	}
  if (dbg_malloc) printf("location+1 = %d\n", location);
  if (dbg_malloc) assert(location+1 <= total_len+1);
	s[location] = '\0';
	if (dbg) printf("add_num_bits: s = %s\n", s);

}


// ----------------------------------------------------------------------
int num_digits(int a[]) 
{
	int count = 0;
  while (a[count] != -1) {
	  count++;
	}
	return count;
}

// ----------------------------------------------------------------------
void destroy_lev_strings(char *s[], int count) 
{
  for (int i = 0; i < count; i++) {
	  free(s[i]);
    s[i] = NULL;
	}
	free(s);
  s = NULL;
}

// ----------------------------------------------------------------------
void destroy_expected_strs(char **s, int count) 
{
  for (int i = 0; i < count; i++) {
	  free(s[i]);
    s[i] = NULL;
	}
	free(s);
  s = NULL;
}

// ----------------------------------------------------------------------
char **create_expected_strs(int bits) 
{
  int count = mypow(2, bits);
	int binary[MAX_BITS] = {-1};
	int i = 0;
	int j = 0;
  int max_len = bits + 1;

  char **strings = malloc(count * sizeof(char *));
  
	for (i = 0; i < count; i++) {
	   strings[i] = malloc((max_len) * sizeof(char));
		 for (j = 0; j < max_len; j++) {
       // printf("count = %d max_len = %d i = %d j = %d\n", count, max_len, i, j);
		   strings[i][j] = '\0';
		 }
	}

	for (i = 0; i < count; i++) {
	  to_binary(i, binary);
		char *s = strings[i];

    // find out if we need leading zeros
		int leading_zeros = (bits - num_digits(binary));
		if (dbg_expected) printf("num_digits(binary) = %d\n", num_digits(binary));
		if (dbg_expected) printf("leading zeros = %d\n", leading_zeros);

	  add_num_bits(s, leading_zeros, 0);

		int index = 0;
		int cur = 0;
		while ((cur = binary[index]) != -1) {
	    add_num_bits(s, 1, cur);
		  if (dbg_expected) printf("strings[%4d] = %s\n", i, strings[i]);
			index++;
		}
    assert(strlen(s) == bits);
	}

	if (dbg) {
	  for (int i = 0; i < count; i++) {
	    printf("expected_str[%4d] = %s\n", i, strings[i]);
      fflush(stdout);
		}
	}
  return strings;
}

// ----------------------------------------------------------------------
char **create_lev_strings(int b, int m, int p, int *count) 
{
	int i = 0;
	int j = 0;
	int cur = 0;
	int binary[MAX_BITS+1] = {-1};
	assert(b <= MAX_BITS);
  *count = mypow(2, b);
	total_len = 2 + (2 * p) + (b * m);
  // TODO: For some reason 1 was causing problems (likely off by 1 error somewhere)
  int max_len = total_len + 1;

	if (total_len > MAX_BITS) {
	
	  printf("total_len for packet = 2 + (2 * p) + (b * m)\n");
	  printf("total_len for packet = 2 + (2 * %d) + (%d * %d) = %d\n",
		        p, b, m, total_len);
		printf("total_len (%d) > MAX_BITS (%d)\n", total_len, MAX_BITS);
    exit(1);
	}

  char **strings = malloc((*count) * sizeof(char *));
   
	for (i = 0; i < *count; i++) {
     // TODO: FIX ME (10 should be 1 but there were occasional crashes and valgrind reported problems it's causing problems)
	   strings[i] = malloc(max_len * sizeof(char));
		 for (j = 0; j < max_len; j++) {
		   strings[i][j] = '\0';
		 }
	}

	for (i = 0; i < *count; i++) {
	   char *s = strings[i];
		// add the 1 for the start of the preamable
	  add_num_bits(s, 1, 1);
    // add p 0's for the rest of the preamable
	  add_num_bits(s, p, 0);


    // add the bits for the number
	  to_binary(i, binary);

    // find out if we need leading zeros
		int leading_zeros = m * (b - num_digits(binary));
		if (dbg) printf("num_digits(binary) = %d\n", num_digits(binary));
		if (dbg) printf("leading zeros = %d\n", leading_zeros);

	  add_num_bits(s, leading_zeros, 0);

		int index = 0;
		while ((cur = binary[index]) != -1) {
	    add_num_bits(s, m, cur);
			index++;
		}

    // add p 0's for the postamable
	  add_num_bits(s, p, 0);
		// add the 1 for the end of the postamble
	  add_num_bits(s, 1, 1);
		// 2 bits for 1 at start and end, 2 * p for pre and postamble + b * m
		// printf("Expected len = %d\n", (2 + (2 * p) + (b * m)));
		// printf("strlen(s) = %lu\n", strlen(s));
	  assert(strlen(s) == total_len);
	}

	if (dbg || print_lev_strings) {
	  for (int i = 0; i < *count; i++) {
	    printf("lev_str[%4d] = %s\n", i, strings[i]);
		}
	}
	return strings;
}

// ----------------------------------------------------------------------
int levenshtein_orig(char *s1, char *s2) 
{
    unsigned int s1len, s2len, x, y, lastdiag, olddiag;
    s1len = strlen(s1);
    s2len = strlen(s2);
    unsigned int column[s1len + 1];
    for (y = 1; y <= s1len; y++)
        column[y] = y;
    for (x = 1; x <= s2len; x++) {
        column[0] = x;
        for (y = 1, lastdiag = x - 1; y <= s1len; y++) {
            olddiag = column[y];
            column[y] = MIN3(column[y] + 1, column[y - 1] + 1, lastdiag + (s1[y-1] == s2[x - 1] ? 0 : 1));
            lastdiag = olddiag;
        }
    }
    return column[s1len];
}

#include "decode.h"

int find_min(int x, int y, int z) {
  if (x <= y && x <= z) {
    return x;
  } else if (y <=x && y <= z) {
    return y;
  } else {
    return z;
  }
}

int levenshtein_new(char a[], char b[]){
  int len_a = strlen(a);
  int len_b = strlen(b);
  // Declaring a 2D array off maximum size
  int d[AMPDU_LEN+1][AMPDU_LEN+1];
  assert(len_a <= AMPDU_LEN);
  assert(len_b <= AMPDU_LEN);

  // Initialising first column:
  for (int i = 0; i < len_a + 1; i++) {
    d[i][0] = i;
  }

  // Initialising first row:
  for (int j = 0; j < len_b + 1; j++) {
    d[0][j] = j;
  }

  // Applying the algorithm:
  int insertion = 0;
  int deletion = 0;
  int replacement = 0;

  for (int i = 1; i < len_a + 1; i++) {
    for (int j = 1; j < len_b + 1; j++) {
      if (a[i - 1] == b[j - 1]) {
        d[i][j] = d[i - 1][j - 1];
      } else {
        // Choosing the best option:
        insertion = d[i][j - 1];
        deletion = d[i - 1][j];
        replacement = d[i - 1][j - 1];

        d[i][j] = 1 + find_min(insertion, deletion, replacement);
      }
    }
  }

  int answer = d[len_a][len_b];
  return answer;
}

#ifdef MAIN
int main() {
  char a[] = "Carry";
  char b[] = "Bark";
  int result = 0;

  result = levenshtein(a, b);
  printf("result = %d\n", result);
  return 0;
}
#endif
