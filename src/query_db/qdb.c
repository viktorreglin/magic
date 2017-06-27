#include <string.h>
#include "qbd.h"
#include "macros.h"
#include "str.h"



static int match( char * srctype, char * typepattern, STR * pparam1 )
{
   char * parpattern;
   STR pat1, pat2, src1, src2;
   int m;

   if( pparam1 )
   {
      (*pparam1).str = "";
      (*pparam1).len = 0;
   }
   parpattern = strchr( typepattern, '%' );
   if( parpattern )
   {
      pat1.str = typepattern;
      pat1.len = parpattern - typepattern;
      pat2.str = parpattern + 2;      // %X
      pat2.len = strlen( pat2.str );  // 1 param only
      scr1.str = srctype;
      src1.len = pat1.len;
      src2.str = srctype + strlen(srctype) - pat2.len;
      src2.len = pat2.len;
      m = compareStrings(src1,pat1) && compareStrings(src2,pat2);
      if( m && pparam1 )
      {
         (*pparam1).str = src1.str + src1.len;
         (*pparam1).len = src2.str - src1.str;
      }
      return m;
   }
   else
   {
      return strcasecmp( srctype, typepattern ) == 0;
   }
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
}
