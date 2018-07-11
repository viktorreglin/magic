#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "port.h"
#include "salloc.h"
#include "filelength.h"
#include "readfile.h"


char * readfile( char * fname )
/* ================================================================ */
{
int f;
int len, flen;
char * store;

   /* open the file and determine length */
   f = open( fname, O_RDONLY | O_BINARY );
   if( f == -1 )
   {
      fprintf(stderr, "Cannot open '%s'\n", fname );
      return 0;
   }

   flen = filelength( f );
   if( flen < 0 )
   {
      fprintf(stderr, "Cannot determine file length of '%s'\n", fname );
      return 0;
   }
   else if( flen == 0 )
   {
      fprintf(stderr, "File '%s' is empty", fname );
      return "";
   }

   // allocate memory
   store = salloc( flen+2 );

   /* read the file: */
   len = read( f, store, flen );
   if( len != flen )
   {
      fprintf(stderr, "Error reading '%s'", fname );
      close( f );
      sfree( store );
      return 0;
   }
   close( f );
   store[len]   = '\n';
   store[len+1] = 0;

   return store;
}
