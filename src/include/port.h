#ifndef _PORT_H_
#define _PORT_H_

/*
   portability stuff
*/

#ifdef WINDOWS
#include <direct.h>
#include <io.h>
#define DIRSEP  '\\'
#define open  _open
#define close _close
#define read  _read
#endif

#ifdef LINUX
#include <unistd.h>
#define O_BINARY 0
#define DIRSEP  '/'
#define stricmp strcasecmp
#endif


#endif /* _PORT_H_ */
