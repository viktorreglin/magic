#ifndef _ASPRINTF_H_
#define _ASPRINTF_H_

char * asprintf( char * format, ... );
/* like printf - but into an allocated buffer, which is returned */

char * astrcpy( char * str );
/* get allocated string copy */

#endif // _ASPRINTF_H_
