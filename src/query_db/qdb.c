#include <string.h>

#include "bool.h"
#include "macros.h"
#include "port.h"
#include "salloc.h"
#include "str.h"

#include "qdb.h"

static bool match( char * srctype, char * typepattern, STR * pparam1 )
{
   char * parpattern;
   STR pat1, pat2, src1, src2;
   bool mtch;

   if( pparam1 )
   {
      (*pparam1).ptr = "";
      (*pparam1).len = 0;
   }
   parpattern = strchr( typepattern, '%' );
   if( parpattern )
   {
      pat1.ptr = typepattern;
      pat1.len = parpattern - typepattern;
      pat2.ptr = parpattern + 2;      // %X
      pat2.len = strlen( pat2.ptr );  // 1 param only
      src1.ptr = srctype;
      src1.len = pat1.len;
      src2.ptr = srctype + strlen(srctype) - pat2.len;
      src2.len = pat2.len;
      mtch = compareStrings(src1,pat1) && compareStrings(src2,pat2);
      if( mtch && pparam1 )
      {
         (*pparam1).ptr = src1.ptr + src1.len;
         (*pparam1).len = src2.ptr - src1.ptr;
      }
      return mtch;
   }
   else
   {
      return stricmp( srctype, typepattern ) == 0;
   }
}


static char * format( char * typepattern, STR param1 )
{
   char * parptr;
   char * str;
   int len1;

   if( !typepattern )
      return "";

   if( param1.len <= 0 )
      return typepattern;

   parptr = strchr( typepattern, '%' );
   if( !parptr )
      return typepattern;

   str = salloc( strlen(typepattern) + param1.len );
   len1 = parptr - typepattern;
   memcpy( str, typepattern, len1 );
   str += len1;
   memcpy( str, param1.ptr, param1.len );
   str += param1.len;
   strcpy( str, parptr+2 ); // %X

   return str;
}


char * sql_type( char * magic_type )
{
   int i;
   STR param1;
   for( i = 0; i < arraysize(typemapping); i++ )
   {
      if( match( magic_type, typemapping[i].magtype, &param1 ) )
         return format( typemapping[i].sqltype, param1 );
   }
   if( stricmp( magic_type, "string" ) == 0 )
      return "text";                 // sql type default
   return sql_type( "string" );      // magic type default
}
