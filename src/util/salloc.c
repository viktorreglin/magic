
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "salloc.h"


void * salloc( int size )
/* ================================================================ */
/* valid pointer is guaranteed, the contents is initialized w 0
   otherwise a fatal error is issued.
*/
{
void * ptr;

   if( size <= 0 )
      return 0;

   ptr = malloc( size );
   if( !ptr )
   {
      fprintf( stderr, "FATAL: Not enough memory (trying to allocate %d bytes)\n", size );
      exit( 9 );
   }
   memset( ptr, 0, size );
   return ptr;
}
