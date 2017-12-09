#ifndef _STR_H
#define _STR_H
// strings mit explizizer Laenge


typedef struct
{
   int    len;
   char * ptr; // muss nicht 0-terminiert sein !
} STR;

int compareLength( STR str1, STR str2 );
int compareChars( STR str1, STR str2 );
int compareStrings( STR str1, STR str2 );

#endif /* _STR_H */
