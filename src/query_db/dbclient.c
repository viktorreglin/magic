// db client, Experimentierversion

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mysql.h>


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
   aus der sql-Datei, mit der die Tabelle erzeugt wurde.
   Wahrscheinlich wird nur gebarucht, ob String oder nicht. */
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


static print_error( MYSQL * conn, char * txt1, char * txt2 )
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


static void insertstrings( char * query, char * pattern, char mark, int num, char * par[], unsigned char * isstring )
{
char * dst;
char * pat;
int i;

   dst = strchr( query, mark );
   pat = strchr( pattern, mark );
   if( !dst || !pat )
   {
      print_error( 0, "internal error", "wrong insert pattern" );
      return;
   }

   for( i = 0; i < num; i++ )
   {
      if( isstring && isstring[i] )
         *dst++ = QUERY_STRING_DELIM;
      strcpy( dst, par[2*i] );
      dst += strlen(par[2*i]);
      if( isstring && isstring[i] )
         *dst++ = QUERY_STRING_DELIM;
      *dst++ = QUERY_PAR_DELIM;
   }
   dst--;
   pat++;
   strcpy( dst, pat );
}


static TABLEENTRY * gettabletypes( char * table )
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


static void insert( MYSQL * conn, char * table, int numofpairs, char * par[] )
{
   int i, qlen;
   char * query;
   char * prop;
   char * val;
   TABLEENTRY * tabletype;
   unsigned char * isstring; // array: 1 element per property
   static char pattern[] = "insert into %s (!) values (?);"; // %s = table, ! = properties, ? = values

   // Wir brauchen die Typen der Properties
   tabletype = gettabletypes( table );
   isstring = malloc(numofpairs);
   if( !isstring )
   {
      print_error( 0, "not enough memory for type info", "" );
      return;
   }
   for( i = 0; i < numofpairs; i++ )
      isstring[i] = typeisstring( par[2*i], tabletype );

   // Gesamt-Stringlaenge bestimmen
   qlen = strlen(pattern) + strlen(table) + 1;
   for( i = 0; i < numofpairs; i++ )
      qlen += strlen(par[2*i]) + strlen(par[2*i+1]) + 4; // 4 for ,,''

   // query bauen
   query = malloc(qlen);
   if( !query )
   {
      print_error( 0, "not enough memory for query", pattern );
      return;
   }
   sprintf( query, pattern, table );
   insertstrings( query, pattern, '!', numofpairs, par  , 0        ); // properties
   insertstrings( query, pattern, '?', numofpairs, par+1, isstring ); // values, type info benutzen
   printf( "dbclient query: %s\n", query );

   // zur Datenbank
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
   free( query );
}


static void dump( MYSQL * conn, char * table )
{
   int i, num, qlen;
   char * query;
   char * patt2;
   TABLEENTRY * pt;
   char ** prop;
   MYSQL_RES * result;
   MYSQL_ROW   row;
   static char pattern[] = "select ! from %s;"; // %s = table, ! = properties

   // Gesamt-Stringlaenge bestimmen
   qlen = strlen(pattern) + strlen(table) + 1;
   num = 0;
   pt = gettabletypes( table );
   while( pt->name )
   {
      qlen += strlen( pt->name ) + 1; // 1 for ,
      num++;
      pt++;
   }
   // property array
   prop = malloc(2*num*sizeof(char *));
   if( !prop )
   {
      print_error( 0, "not enough memory for property array", table );
      return;
   }
   num = 0;
   pt = gettabletypes( table );
   while( pt->name )
   {
      prop[2*num] = pt->name;
      num++;
      pt++;
   }

   // query bauen
   query = malloc(qlen);
   if( !query )
   {
      print_error( 0, "not enough memory for query", pattern );
      return;
   }
   sprintf( query, pattern, table );
   patt2 = malloc(strlen(query)+1);
   if( !patt2 )
   {
      print_error( 0, "not enough memory for pattern copy", query );
      return;
   }
   patt2 = strcpy( patt2, query );

   insertstrings( query, patt2, '!', num, prop, 0 );
   printf( "dbclient query: %s\n", query );

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
   free(prop);
   free(query);
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
      insert( conn, argv[2], (argc-3)/2, argv+3 );
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
