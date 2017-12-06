#include <string.h>

#include "asprintf.h"
#include "bool.h"
#include "macros.h"
#include "port.h"
#include "salloc.h"
#include "str.h"

#define TYPEMAPPING_DEFINITION
#include "qdb.h"

#define QUERY_STRING_DELIM '\''
#define QUERY_PAR_DELIM    ','


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
         (*pparam1).len = src2.ptr - (*pparam1).ptr;
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
   char * s;
   int len1;

   if( !typepattern )
      return "";

   if( param1.len <= 0 )
      return typepattern;

   parptr = strchr( typepattern, '%' );
   if( !parptr )
      return typepattern;

   str = salloc( strlen(typepattern) + param1.len );
   s = str;
   len1 = parptr - typepattern;
   memcpy( s, typepattern, len1 );
   s += len1;
   memcpy( s, param1.ptr, param1.len );
   s += param1.len;
   strcpy( s, parptr+2 ); // %X

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


void mysql_print_error( MYSQL * conn, char * txt1, char * txt2 )
{
   fprintf( stderr, "ERROR: %s %s\n", txt1, txt2 );
   if( conn )
      fprintf( stderr, "mysql: %s\n", mysql_error(conn) );
}


QDB_HANDLE qdb_open( char * dbname )
{
   MYSQL * conn;

   static char * s_host    = NULL;
   static char * s_user    = "root";
   static char * s_passwd  = NULL;
   static char * s_socket  = NULL;
   static unsigned int s_port = 0;


   if( mysql_library_init( 0, NULL, NULL) )
   {
      mysql_print_error( NULL, "cannot initialize mysql library", "" );
      return 0;
   }
   conn = mysql_init( NULL );
   if( !conn )
   {
      mysql_print_error( NULL, "cannot initialize mysql connection handler", "" );
      return 0;
   }
   if( !mysql_real_connect( conn, s_host, s_user, s_passwd, dbname, s_port, s_socket, 0 ) )
   {
      mysql_print_error( conn, "cannot connect to mysql db", dbname );
      qdb_close( &conn );
      return 0;
   }

   return conn;
}


void qdb_close( QDB_HANDLE * pqh )
{
   mysql_close( *pqh );
   mysql_library_end();
   *pqh = 0;
}


static bool typeisstring( char * typ )
{
   int i;
   for( i = 0; i < arraysize(stringtypes); i++ )
   {
      if( strcmp(typ,stringtypes[i]) == 0 )
         return true;
   }
   return false;
}


QDB_TABLEENTRY * qdb_get_properties( char * db, char * table, int * pnum )
{
   static int maxtablerows = 100;
   static char query[300];
   int r, nfields;
   MYSQL * conn;
   MYSQL_RES * result;
   MYSQL_ROW   row;
   QDB_TABLEENTRY * proptab;

   conn = qdb_open( "information_schema" );
   if( !conn )
      return 0;

   sprintf( query, "select column_name, data_type from columns where table_schema='%s' and table_name='%s';", db, table );

   if( mysql_query( conn, query ) != 0 )
   {
      mysql_print_error( conn, "cannot get meta data", query );
      return 0;
   }
   else
   {
      result = mysql_store_result(conn);
      if( result )
      {
         proptab = salloc( (maxtablerows+1) * sizeof(QDB_TABLEENTRY) );

         nfields = mysql_num_fields(result);
         if( nfields != 2 )
         {
            printf( "qdb_get_properties: internal error" );
            exit( 3 );
         }
         r = 0;
         while( (row = mysql_fetch_row(result)) != NULL )
         {
            if( r < maxtablerows )
            {
               proptab[r].name     = astrcpy( row[0] );
               proptab[r].sqltype  = astrcpy( row[1] );
               proptab[r].isstring = typeisstring(proptab[r].sqltype);
               proptab[r].value    = 0;
            }
            r++;
         }
         mysql_free_result(result);
         if( r < maxtablerows )
         {
            proptab[r].name     = 0;
            proptab[r].sqltype  = 0;
            proptab[r].isstring = false;
            proptab[r].value    = 0;
         }
      }
      else
      {
         mysql_print_error( conn, "cannot store result", query );
         return 0;
      }
   }

   qdb_close( &conn );
   if( r <= maxtablerows )
   {
      // printf( "%d properties (max=%d)\n", r, maxtablerows );;;
      if( pnum )
         *pnum = r;
      if( r == 0 )
      {
         sfree(proptab);
         printf( "database/table %s/%s not found\n", db, table );
         return 0;
      }
      return proptab;
   }
   sfree(proptab);
   maxtablerows = 2 * maxtablerows;
   return qdb_get_properties( db, table, pnum );
}


static int qstrcpy( char * dst, char * src ) // ' --> '', returns # of such extra chars
{
   int add = 0;
   if( !src )
      return 0;
   while( *src )
   {
      if( *src == QUERY_STRING_DELIM )
      {
         add++;
         if( dst )
            *dst++ = QUERY_STRING_DELIM;
      }
      if( dst )
         *dst++ = *src;
      src++;
   }
   return add;
}


static int setvalcpy( char * dst, char * src ) // commata zw. elementen, returns # of such extra chars
// unschoen: Wir >>wissen<< hier, dass Elemente nur 1 Zeichen lang sind ....
{
   int add = 0;
   if( !src )
      return 0;

   if( dst )
      *dst++ = *src;
   src++;

   while( *src )
   {
      if( dst )
         *dst++ = ',';
      add++;

      if( dst )
         *dst++ = *src;
      src++;
   }

   return add;
}


static void insertnames( char * query, char * pattern, char mark, int num, QDB_TABLEENTRY * prop )
{
char * dst;
char * pat;
int i, x;

   dst = strchr( query, mark );
   pat = strchr( pattern, mark );
   if( !dst || !pat )
      return; // nothing to do

   for( i = 0; i < num; i++ )
   {
      x = qstrcpy( dst, prop[i].name );
      dst += strlen(prop[i].name) + x;
      *dst++ = QUERY_PAR_DELIM;
   }
   dst--;
   pat++;
   strcpy( dst, pat );
}


static void insertvalues( char * query, char * pattern, char mark, int num, QDB_TABLEENTRY * prop )
{
char * dst;
char * pat;
int i, x;

   dst = strchr( query, mark );
   pat = strchr( pattern, mark );
   if( !dst || !pat )
      return; // nothing to do

   for( i = 0; i < num; i++ )
   {
      if( prop[i].isstring )
         *dst++ = QUERY_STRING_DELIM;
      if( strcmp(prop[i].sqltype,"set") == 0 )
         x = setvalcpy( dst, prop[i].value );  // fuegt , zw. Elementen ein
      else
         x = qstrcpy( dst, prop[i].value );
      dst += strlen(prop[i].value) + x;
      if( prop[i].isstring )
         *dst++ = QUERY_STRING_DELIM;
      *dst++ = QUERY_PAR_DELIM;
   }
   dst--;
   pat++;
   strcpy( dst, pat );
}


char * sql_buildquery( char * pattern, char * table, QDB_TABLEENTRY * prop, int num, bool withvalues )
{
   int i, qlen;
   char * patt2; // pattern w table
   char * query;

   qlen  = strlen(pattern) + strlen(table) + 1;
   patt2 = salloc(qlen+1);
   sprintf( patt2, pattern, table );

   for( i = 0; i < num; i++ )
      qlen += strlen(prop[i].name) + 1; // 1 for ,

   if(withvalues)
   {
      for( i = 0; i < num; i++ )
      {
         if( prop[i].value == 0 )
         {
            prop[i].value = prop[i].isstring ? "" : "0";
         }
         qlen += strlen(prop[i].value) + qstrcpy( 0, prop[i].value ) + 3; // 3 for ,''
      }
   }

   query = salloc(qlen+1);
   strcpy(query,patt2);
   insertnames( query, patt2, '!', num, prop );
   if( withvalues )
      insertvalues( query, patt2, '?', num, prop );

   // printf( "*** query: %s\n", query );;;
   sfree( patt2 );
   return query;
}

