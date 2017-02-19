#ifndef _TEST_H_
#define _TEST_H_

#include <string.h>

static int test_result = 0;

#define TEST_EQ(EXPRESSION,VALUE) \
{\
   if( (EXPRESSION) == (VALUE) ) \
      printf( "OK: " #EXPRESSION " == " #VALUE " (%d)\n", (VALUE) ); \
   else \
   {\
      printf( "ERROR: " #EXPRESSION " is %d != " #VALUE " (%d)\n", (EXPRESSION), (VALUE) ); \
      test_result++;\
   }\
}

#define TEST_EQ_STR(EXPRESSION,STRING) \
{\
   if( strcmp((EXPRESSION),(STRING)) == 0 ) \
      printf( "OK: " #EXPRESSION " == '" STRING "'\n" ); \
   else \
   {\
      printf( "ERROR: " #EXPRESSION " is '%s' != '" STRING "'\n", (EXPRESSION) ); \
      test_result++;\
   }\
}


#endif /* _TEST_H_ */
