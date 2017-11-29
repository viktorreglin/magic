#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asprintf.h"
#include "salloc.h"

#define STRMAX 4000

char * asprintf( char * format, ... )
{
static char buf[STRMAX];
va_list ap;
char * newbuf;
int bytes;

   va_start( ap, format );
   bytes = vsprintf( buf, format, ap );
   if( bytes >= STRMAX )
   {
      printf( "FATAL: internal error: string too long (%d > %d)\n", bytes, STRMAX );
      exit( 9 );
   }
   va_end( ap );
   newbuf = salloc( bytes + 1 );
   strcpy( newbuf, buf );
   return newbuf;
}


char * astrcpy( char * str )
{
char * newbuf;
int bytes;

   if( !str )
      str = "";

   bytes = strlen(str);
   newbuf = salloc( bytes + 1 );
   strcpy( newbuf, str );
   return newbuf;
}

