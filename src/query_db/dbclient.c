// db client, Experimentierversion

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mysql.h>

#include "bool.h"
#include "salloc.h"

typedef struct
{
   char * name;
   char * type;
   int typeflags;
} TABLEENTRY;

// typeflags:
#define TYPE_IS_STRING 1


#define QUERY_STRING_DELIM '\''
#define QUERY_PAR_DELIM    ','

// diese Daten sollten aus dem config-File kommen
static char * s_host    = NULL;
static char * s_user    = "root";
static char * s_passwd  = NULL;
static char * s_dbname  = "magic1";
static char * s_socket  = NULL;
static unsigned int s_port = 0;

/* Diese Information sollte aus der Datenbank kommen oder
   aus der sql-Datei, mit der die Tabelle erzeugt wurde. */
static TABLEENTRY s_table[] =
{
   { "name"         , "varchar(80)", 1 },
   { "pricecent"    , "int"        , 0 },
   { "edition"      , "char(8)"    , 1 },
   { "colorIdentity", "varchar(15)", 1 },
   { "id"           , "char(40)"   , 1 },
   { "manaCost"     , "smallint"   , 0 },
   { "types"        , "varchar(40)", 1 },
   { "power"        , "tinyint"    , 0 },
   { "toughness"    , "tinyint"    , 0 },
   { "text"         , "text"       , 1 },
   { 0, 0, 0 } // end of table
};


static void usage( void )
{
   fprintf( stderr, "usage: dbclient option [parameters]\n" );
   fprintf( stderr, "       options: -i  insert data\n" );
   fprintf( stderr, "                -c  change data\n" );
   fprintf( stderr, "                -r  remove data\n" );
   fprintf( stderr, "                -d  dump data\n" );
   fprintf( stderr, "       dbclient -i table property value property value ...\n" );
   fprintf( stderr, "       dbclient -d table\n" );
   exit( 2 );
}


static void print_error( MYSQL * conn, char * txt1, char * txt2 )
{
   fprintf( stderr, "ERROR: %s %s\n", txt1, txt2 );
   if( conn )
      fprintf( stderr, "mysql: %s\n", mysql_error(conn) );
}


static void disconnect( MYSQL * conn )
{
   mysql_close( conn );
   mysql_library_end();
}


static MYSQL * connect( void )
{
MYSQL * conn;

   if( mysql_library_init( 0, NULL, NULL) )
   {
      print_error( NULL, "cannot initialize mysql library", "" );
      return 0;
   }
   conn = mysql_init( NULL );
   if( !conn )
   {
      print_error( NULL, "cannot initialize mysql connection handler", "" );
      return 0;
   }
   if( !mysql_real_connect( conn, s_host, s_user, s_passwd, s_dbname, s_port, s_socket, 0 ) )
   {
      print_error( conn, "cannot connect to mysql db", s_dbname );
      disconnect( conn );
      return 0;
   }

   return conn;
}


static TABLEENTRY * gettabledefinition( char * table )
{
   return s_table; // spaeter aus db oder  db_table.sql
}


static unsigned char typeisstring( char * elemname, TABLEENTRY * typeinfo )
{
TABLEENTRY * pt;

   pt = typeinfo;
   while( pt->name )
   {
      if( strcmp(pt->name,elemname) == 0 )
         return pt->typeflags & TYPE_IS_STRING;
      pt++;
   }
   print_error( 0, "property does not exist", elemname );
   return 0;
}


static char ** propertyarray( char * table, int * pnum )
{
   int num, n;
   TABLEENTRY * pt;
   TABLEENTRY * qt;
   char ** prop;

   qt = gettabledefinition( table );
   // Anzahl bestimmen
   num = 0;
   pt = qt;
   while( pt->name )
   {
      num++;
      pt++;
   }
   // property array
   prop = salloc(num*sizeof(char *));
   n = 0;
   pt = qt;
   while( pt->name )
   {
      prop[n] = pt->name;
      n++;
      pt++;
   }
   if( pnum )
      *pnum = num;
   return prop;
}


static void insertstrings( char * query, char * pattern, char mark, int num, char * par[], bool isstring[] )
{
char * dst;
char * pat;
int i;

   dst = strchr( query, mark );
   pat = strchr( pattern, mark );
   if( !dst || !pat )
      return; // nothing to do

   for( i = 0; i < num; i++ )
   {
      if( isstring && isstring[i] )
         *dst++ = QUERY_STRING_DELIM;
      strcpy( dst, par[i] );
      dst += strlen(par[i]);
      if( isstring && isstring[i] )
         *dst++ = QUERY_STRING_DELIM;
      *dst++ = QUERY_PAR_DELIM;
   }
   dst--;
   pat++;
   strcpy( dst, pat );
}


static char * buildquery( char * pattern, char * table, TABLEENTRY * tabdef, int num, char * prop[], char * val[] )
{
   int i, qlen;
   char * patt2; // pattern w table
   char * query;
   bool * stringpar;

   qlen  = strlen(pattern) + strlen(table) + 1;
   patt2 = salloc(qlen+1);
   sprintf( patt2, pattern, table );

   for( i = 0; i < num; i++ )
      qlen += strlen(prop[i]) + 1; // 1 for ,

   if(val)
   {
      for( i = 0; i < num; i++ )
         qlen += strlen(val[i]) + 3; // 3 for ,''
   }

   query = salloc(qlen+1);
   strcpy(query,patt2);
   insertstrings( query, patt2, '!', num, prop, 0 );
   if( val )
   {
      stringpar = salloc( num * sizeof(bool) );
      for( i = 0; i < num; i++ )
         stringpar[i] = typeisstring( prop[i], tabdef );
      insertstrings( query, patt2, '?', num, val, stringpar );
      sfree(stringpar);
   }
   printf( "dbclient query: %s\n", query );;;
   sfree( patt2 );
   return query;
}


static void insert( MYSQL * conn, char * table, int numofpairs, char * prop[], char * val[] )
{
   char * query;
   TABLEENTRY * tabdef;
   static char pattern[] = "insert into %s (!) values (?);"; // %s = table, ! = properties, ? = values

   tabdef = gettabledefinition( table );
   query  = buildquery( pattern, table, tabdef, numofpairs, prop, val );
   if( mysql_query( conn, query ) != 0 )
   {
      print_error( conn, "cannot insert", query );
   }
   else
   {
      unsigned long n;
      n = (unsigned long)mysql_affected_rows(conn);
      printf( "dbclient: %lu affected row%s\n", n, n == 1 ? "" : "s" );
   }
   sfree( query );
}


static void insert2( MYSQL * conn, char * table, int numofpairs, char * par[] )
{
   /* nur Umsortieren der Parameter:
     par[0] -> prop[0]
     par[1] -> val[0]
     ...
   */
   int i;
   char ** prop;
   char ** val;

   prop = par;
   val = salloc( numofpairs * sizeof(char*) );
   for( i = 0; i < numofpairs; i++ )
   {
      prop[i] = par[2*i];
      val [i] = par[2*i+1];
   }

   insert( conn, table, numofpairs, prop, val );
   sfree( val );
}


static void dump( MYSQL * conn, char * table )
{
   char * query;
   char ** prop;
   int i, num;
   MYSQL_RES * result;
   MYSQL_ROW   row;
   static char pattern[] = "select ! from %s;"; // %s = table, ! = properties

   prop = propertyarray( table, &num );
   query = buildquery( pattern, table, 0, num, prop, 0 );

   // zur Datenbank
   if( mysql_query( conn, query ) != 0 )
   {
      print_error( conn, "cannot dump", query );
   }
   else
   {
      result = mysql_store_result(conn);
      if( result )
      {
         while( (row = mysql_fetch_row(result)) != NULL )
         {
            for( i = 0; i < mysql_num_fields(result); i++ )
            {
               printf( "%s\t", row[i] );
            }
            printf("\n");
         }

         mysql_free_result(result);
      }
      else
      {
         print_error( conn, "cannot store result", query );
      }
   }

   sfree( query );
   sfree(prop);
}


int main( int argc, char * argv[] )
{
MYSQL * conn;

   conn = connect();
   if( !conn )
      exit( 1 );

   if( argc < 2 )
      usage();
   if( argv[1][0] != '-' )
      usage();
   switch( argv[1][1] )
   {
   case 'i':
      if( argc < 5 )
         usage();
      if( argc % 2 != 1 )
         usage();
      insert2( conn, argv[2], (argc-3)/2, argv+3 );
      break;
   case 'c':
      print_error( 0, "not implemented, yet", "-c" );
      break;
   case 'r':
      print_error( 0, "not implemented, yet", "-r" );
      break;
   case 'd':
      if( argc != 3 )
         usage();
      dump( conn, argv[2] );
      break;
   default:
      usage();
   }

   disconnect(conn);
   exit(0);
}
