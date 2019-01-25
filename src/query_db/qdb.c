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

#define MAX_TABLE_NAME_LENGTH 100

extern char * strcasestr( char *, char * );


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


void sql_print_error( MYSQL * conn, char * txt1, char * txt2 )
{
   fprintf( stderr, "ERROR: %s %s\n", txt1, txt2 );
   if( conn )
      fprintf( stderr, "mysql: %s\n", mysql_error(conn) );
}


MYSQL * sql_open( char * dbname )
{
   MYSQL * conn;

   static char * s_host    = NULL;
   static char * s_user    = "root";
   static char * s_passwd  = NULL;
   static char * s_socket  = NULL;
   static unsigned int s_port = 0;


   if( mysql_library_init( 0, NULL, NULL) )
   {
      sql_print_error( NULL, "cannot initialize mysql library", "" );
      return 0;
   }
   conn = mysql_init( NULL );
   if( !conn )
   {
      sql_print_error( NULL, "cannot initialize mysql connection handler", "" );
      return 0;
   }
   if( !mysql_real_connect( conn, s_host, s_user, s_passwd, dbname, s_port, s_socket, 0 ) )
   {
      sql_print_error( conn, "cannot connect to mysql db", dbname );
      sql_close( conn );
      return 0;
   }

   return conn;
}


void sql_close( MYSQL * dh )
{
   mysql_close( dh );
   mysql_library_end();
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

   conn = sql_open( "information_schema" );
   if( !conn )
      return 0;

   sprintf( query, "select column_name, data_type from columns where table_schema='%s' and table_name='%s';", db, table );

   if( mysql_query( conn, query ) != 0 )
   {
      sql_print_error( conn, "cannot get meta data", query );
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
            fprintf( stderr, "qdb_get_properties: internal error" );
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
               proptab[r].isdate   = strcasecmp(proptab[r].sqltype, "datetime") == 0;
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
            proptab[r].isdate   = false;
            proptab[r].value    = 0;
         }
      }
      else
      {
         sql_print_error( conn, "cannot store result", query );
         return 0;
      }
   }

   sql_close( conn );
   if( r <= maxtablerows )
   {
      // printf( "%d properties (max=%d)\n", r, maxtablerows );;;
      if( pnum )
         *pnum = r;
      if( r == 0 )
      {
         sfree(proptab);
         fprintf( stderr, "database/table %s/%s not found\n", db, table );
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

   if( !*src )
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
      bool nonenulldate = false;
      if( prop[i].isdate )
      {
         if( strcmp(prop[i].value,"0") == 0 )
            prop[i].value = astrcpy("current_timestamp()");
         else
            nonenulldate = true;
      }
      if( prop[i].isstring  || nonenulldate )
         *dst++ = QUERY_STRING_DELIM;
      if( strcmp(prop[i].sqltype,"set") == 0 )
         x = setvalcpy( dst, prop[i].value );  // fuegt , zw. Elementen ein
      else
         x = qstrcpy( dst, prop[i].value );
      dst += strlen(prop[i].value) + x;
      if( prop[i].isstring  || nonenulldate )
         *dst++ = QUERY_STRING_DELIM;
      *dst++ = QUERY_PAR_DELIM;
   }
   dst--;
   pat++;
   strcpy( dst, pat );
}


char *  sql_buildquery( char * pattern, char * p1, char * p2, QDB_TABLEENTRY * prop, int num, bool withvalues )
{
   int i, qlen;
   char * patt2; // pattern w parameters
   char * query;

   if( !p1 )
      p1 = "";
   if( !p2 )
      p2 = "";
   qlen  = strlen(pattern) + strlen(p1) + strlen(p2) + 1;
   patt2 = salloc(qlen+1);
   sprintf( patt2, pattern, p1, p2 );

   for( i = 0; i < num; i++ )
      qlen += strlen(prop[i].name) + 1; // 1 for ,

   if(withvalues)
   {
      for( i = 0; i < num; i++ )
      {
         if( prop[i].value == 0 )
            prop[i].value = prop[i].isstring ? "" : astrcpy("0");

         if( prop[i].isdate && (strlen(prop[i].value) <= 1) )
            qlen += 19; // current_timestamp()

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


static int fillvalue( QDB_TABLEENTRY * properties, char * name, char * value )
// trage name/value in die Tabelle ein, return: 1 if eingetragen, sonst 0
{
   QDB_TABLEENTRY * prop;

   prop = properties;
   while( prop->name )
   {
      if( strcmp( prop->name, name ) == 0 )
      {
         if( prop->value )
         {
            fprintf( stderr, "WARNIMG: property %s has already a value (%s), ignoring new value(%s)\n", name, prop->value, value );
            return 0;
         }
         if( value[0] == 0 ) // ""
            prop->value = value;
         else
            prop->value = astrcpy(value);
         return 1;
      }
      prop++;
   }
   fprintf( stderr, "ERROR: property %s not found\n", name );
   return 0;
}


static unsigned long insert( char * db, char * table, QDB_TABLEENTRY * properties, int numofprop )
{
   unsigned long n;
   char * query;
   MYSQL * conn;
   static char pattern[] = "insert into %s (!) values (?);"; // %s = table, ! = properties, ? = values

   conn = sql_open( db );
   if( !conn )
      return 0;

   query = sql_buildquery( pattern, table, 0, properties, numofprop, true );

   // zur Datenbank
   if( mysql_query( conn, query ) != 0 )
   {
      n = 0;
      sql_print_error( conn, "cannot: ", query );
   }
   else
   {
      n = (unsigned long)mysql_affected_rows(conn);
      // fprintf( stderr, "%lu affected row%s\n", n, n == 1 ? "" : "s" );
   }

   sfree( query );
   sql_close( conn );
   return n;
}


QDB_ROW qdb_begin_row( char * dbname, char * table )
{
   QDB_ROWDATA * prow;

   if( !dbname || !*dbname || !table || !*table )
      return 0;

   prow = salloc(sizeof(QDB_ROWDATA));
   prow->properties = qdb_get_properties( dbname, table, &(prow->numofprop) );
   if( (prow->properties == 0) || (prow->numofprop <= 0) )
   {
      sfree(prow);
      return 0;
   }
   prow->dbname    = astrcpy(dbname);
   prow->tablename = astrcpy(table);
   return prow;
}


bool qdb_end_row( QDB_ROW tr )
{
   QDB_TABLEENTRY * prop;
   unsigned long rowsaffected = 0;

   if( !tr )
      return false;

   rowsaffected = insert( tr->dbname, tr->tablename, tr->properties, tr->numofprop );

   prop = tr->properties;
   while( prop->name )
   {
      if( prop->value && prop->value[0] )
         sfree( prop->value );
      prop++;
   }

   sfree(tr->tablename);
   sfree(tr->dbname);
   sfree(tr->properties);
   sfree(tr);
   return rowsaffected == 1;
}


bool qdb_set_value( QDB_ROW tr, char * property, char * value )
{
   if( !tr || !property || !value )
      return false;

   return fillvalue( tr->properties, property, value ) == 1;
}


static QDB_RESULT * qdb_alloc( int rows, int fields )
{
   int i, k;
   QDB_RESULT * pr;

   pr = salloc( sizeof(QDB_RESULT) );
   pr->numofrows = rows;
   pr->row = salloc( sizeof(QDB_RESULTROW) * (rows+1) );
   for( i = 0; i <= rows; i++ )
   {
      pr->row[i].numofvalues = fields;
      pr->row[i].value = salloc( sizeof(QDB_VALUE) * fields );
      for( k = 0; k < fields; k++ )
      {
         pr->row[i].value[k].strval = 0;
         pr->row[i].value[k].intval = 0;
      }
   }
   return pr;
}


void qdb_free( QDB_RESULT * pr )
{
   int i;

   if( !pr )
      return;

   if( pr->row )
   {
      for( i = 0; i <= pr->numofrows; i++ )
      {
         if( pr->row[i].value )
         {
            sfree( pr->row[i].value );
            pr->row[i].value = 0;
         }
         pr->row[i].numofvalues = 0;
      }
      sfree( pr->row );
      pr->row = 0;
   }
   pr->numofrows = 0;
   sfree(pr);
}


static char * allocate_buffer( char * oldbuf, int * buflen, int minlen )
{
   char * newbuf;

   if( !oldbuf )
   {
      newbuf = salloc(minlen);
   }
   else if( *buflen < minlen )
   {
      newbuf = salloc(minlen);
      memcpy( newbuf, oldbuf, *buflen );
      sfree( oldbuf );
   }
   else
   {
      newbuf = oldbuf;
   }
   *buflen = minlen;
   newbuf[*buflen] = 0;
   return newbuf;
}


char * convertlike( char * psrc, int * srclen )
{
   static char * buf = 0;
   static int buflen = 0;

   bool escape_used;
   char * pe;
   char * pd;
   char * ps = psrc;

   while( *ps && (*ps != '\'') )
      ps++;        // goto string start

   *srclen = ps - psrc;
   if( ! *ps )
      return psrc;

   ps++;
   pe = ps;
   while( *pe && (*pe != '\'') )
      pe++;        // goto string end

   buf = allocate_buffer( buf, &buflen, 2*(pe - psrc)+11 ); // 11 for escape ..
   pd = buf;
   escape_used = false;
   ps = psrc;
   while( ps != pe )
   {
      switch( * ps )
      {
      case '*':
         *pd++ = '%';
          ps++;
         break;
      case '?':
         *pd++ = '_';
          ps++;
         break;
      case '%':
      case '_':
      case '\\':
         *pd++ = '\\';
         *pd++ = *ps++;
         escape_used = true;
         break;
      default:
         *pd++ = *ps++;
      }
   }
   *pd = 0;
   if( escape_used )
      strcpy( pd, " escape '\\' " );

   *srclen = ps - psrc;
   return buf;
}


static char * build_where( char * filter, char * table )
{  // in der ersten Version fast unveraendert lassen, dafuer nur Namen aus 'table' zugelassen (noch keine Joins)
   // Aenderungen : table.name --> name; like '*?%_\' -> like '%_\%\_\\' escape '\';
   static char * buf = 0;
   static int buflen = 0;

   char * pdst;
   char * psrc;
   char * ptable;
   char * plike;
   char * slike;
   char tabledot[MAX_TABLE_NAME_LENGTH + 2];
   int tabledotlen, destlen, copylen, slen;

   if( strlen(table) > MAX_TABLE_NAME_LENGTH )
   {
      fprintf( stderr, "internal error in qdb_query: table name '%s' too long (> %d chars)\n", table, MAX_TABLE_NAME_LENGTH );
      return "";
   }
   strcpy( tabledot, table );
   strcat( tabledot, "." );
   tabledotlen = strlen(tabledot);

   buf = allocate_buffer( buf, &buflen, 2*strlen(filter)+1 );
   pdst = buf;
   destlen = buflen;
   psrc = filter;
   ptable = strcasestr( psrc, tabledot );
   plike  = strcasestr( psrc, "like");
   while( *psrc )
   {
      bool dotable = false;
      bool dolike  = false;
      if( ptable && plike )
      {
         dotable = ptable <= plike;
         dolike  = !dotable;
      }
      else if( ptable )
      {
         dotable = true;
      }
      else if( plike )
      {
         dolike = true;
      }
      else
      {
         copylen = strlen(psrc);
         memcpy( pdst, psrc, copylen );
         pdst += copylen;
         destlen -= copylen;
         psrc += copylen;
      }

      if( dotable )
      {
         copylen = ptable - psrc;
         memcpy( pdst, psrc, copylen );
         pdst += copylen;
         destlen -= copylen;
         psrc = ptable + tabledotlen;
         ptable = strcasestr( psrc, tabledot );
      }
      if( dolike )
      {
         copylen = plike - psrc + 4;
         memcpy( pdst, psrc, copylen );
         pdst += copylen;
         destlen -= copylen;
         psrc += copylen;
         slike = convertlike( psrc, &copylen );
         psrc += copylen;
         slen = strlen( slike );
         if( slen >= destlen )
         {
            *pdst = 0;
            buf = allocate_buffer( buf, &buflen, buflen+2*slen );
            pdst = buf + strlen(buf);
            destlen = buflen - (pdst-buf);
         }
         memcpy( pdst, slike, slen );
         pdst += slen;
         destlen -= slen;
         plike  = strcasestr( psrc, "like");
      }
   }
   return buf;
}


QDB_RESULT * qdb_query( char * dbname, char * table, int * nrows, char * filter )
{
   MYSQL * conn;
   MYSQL_RES * result;
   MYSQL_ROW   row;
   int i, r, nfields, numofprop;
   char * query;
   QDB_TABLEENTRY * properties;
   QDB_RESULT * pres;
   static char pattern[] = "select ! from %s where %s;"; // ! = properties

   if( !nrows )
      return 0;

   *nrows = -1; // error

   if( !dbname || !table || !filter )
      return 0;

   conn = sql_open( dbname );
   if( !conn )
      return 0;


   properties = qdb_get_properties( dbname, table, &numofprop );
   if( !properties )
      return 0;

   query = sql_buildquery( pattern, table, build_where(filter,table), properties, numofprop, false );

   // zur Datenbank
   if( mysql_query( conn, query ) != 0 )
   {
      sql_print_error( conn, "cannot: ", query );
      sfree(properties);
      return 0;
   }
   else
   {
      result = mysql_store_result(conn);
      if( result )
      {
         *nrows = mysql_num_rows(result);
         if( *nrows == 0 )
         {
            sfree(properties);
            return 0;   // no data
         }

         nfields = mysql_num_fields(result);
         if( nfields != numofprop )
         {
            fprintf( stderr, "internal error in qdb_query: %d properties but %d fields (should be equal)\n", numofprop, nfields );
            sfree(properties);
            return 0;
         }

         pres = qdb_alloc( *nrows, nfields );
         // row[0]:
         for( i = 0; i < nfields; i++ )
         {
            pres->row[0].value[i].strval = properties[i].name;
            pres->row[0].value[i].intval = 0;
            if( properties[i].isstring )
               pres->row[0].value[i].intval |= QDB_TYPE_IS_STRING;
            if( properties[i].isdate )
               pres->row[0].value[i].intval |= QDB_TYPE_IS_DATE;
         }
         // row[1]..row[numofrows]:
         r = 0;
         while( (row = mysql_fetch_row(result)) != NULL )
         {
            r++;
            for( i = 0; i < nfields; i++ )
            {
               if( row[i] == 0 )
                  row[i] = "";
               pres->row[r].value[i].strval = row[i];
               pres->row[r].value[i].intval = strtol( row[i], 0, 0 );
               if( strcasecmp(row[i],"true") == 0 ) //???
                  pres->row[r].value[i].intval = 1;
               if( pres->row[0].value[i].intval & QDB_TYPE_IS_DATE )
               {
                  pres->row[r].value[i].intval = 99;;; //@@@ Umrechnung Datumsstring auf Zahl fehlt hier
               }
            }
         }
         mysql_free_result(result);
      }
      else
      {
         sql_print_error( conn, "cannot store result", query );
      }
   }
   sfree(properties);
   sfree( query );
   sql_close( conn );

   return pres;
}

