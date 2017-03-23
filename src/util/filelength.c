
#include "port.h"
#include "filelength.h"


long filelength( int fh )
/* ================================================================ */
{
long current, ende;

   current = lseek( fh, 0L, SEEK_CUR );
   ende    = lseek( fh, 0L, SEEK_END );
   lseek( fh, current, SEEK_SET );
   return ende;
}
