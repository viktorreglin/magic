#ifndef _SALLOC_H_
#define _SALLOC_H_

#include <stdlib.h>

void * salloc( int size );
/* valid pointer is guaranteed, the contents is initialized w 0
   otherwise a fatal error is issued.
*/

#define sfree  free

#endif /* _SALLOC_H_ */
