// db client, Experimentierversion

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mysql.h>

#define QUERY_PAR_DELIM ','

// diese Daten sollten aus dem config-File kommen
static char * s_host    = NULL;
static char * s_user    = "root";
static char * s_passwd  = NULL;
static char * s_dbname  = "magic1";
static char * s_socket  = NULL;
static unsigned int s_port = 0;

static char * s_table   = "card";



static void usage( void )
{
   fprintf( stderr, "usage: dbclient option [parameters]\n" );
   fprintf( stderr, "       options: -t  create or change table\n" );
   fprintf( stderr, "                -x  remove table\n" );
   fprintf( stderr, "                -i  insert data\n" );
   fprintf( stderr, "                -c  change data\n" );
   fprintf( stderr, "                -r  remove data\n" );
   fprintf( stderr, "                -d  dump database\n" );
   fprintf( stderr, "       dbclient -t table property type property type ...\n" );
   fprintf( stderr, "       dbclient -x table\n" );
   fprintf( stderr, "       dbclient -i table property value property value ...\n" );
   fprintf( stderr, "       dbclient -d\n" );
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


static void insertstrings( char * query, char * pattern, char mark, int num, char * par[] )
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
      strcpy( dst, par[2*i] );
      dst += strlen(par[2*i]);
      *dst++ = QUERY_PAR_DELIM;
   }
   dst--;
   pat++;
   strcpy( dst, pat );
}


static void createtable( MYSQL * conn, char * table, int numofpairs, char * par[] )
{
   print_error( 0, "not implemented, yet", "-t" );
}


static void deletetable( MYSQL * conn, char * table )
{
   print_error( 0, "not implemented, yet", "-x" );
}


static void insert( MYSQL * conn, char * table, int numofpairs, char * par[] )
{
   int i, qlen;
   char * query;
   char * prop;
   char * val;
   static char pattern[] = "insert into %s (!) values (?);"; // %s = table, ! = properties, ? = values

   // Gesamt-Stringlaenge bestimmen
   qlen = strlen(pattern) + strlen(table) + 1;
   for( i = 0; i < numofpairs; i++ )
      qlen += strlen(par[2*i]) + strlen(par[2*i+1]) + 4; // 4 for ,,''

   // query bauen
   query = malloc(qlen);
   if( !query )
   {
      print_error( 0, "not enough memory for query", "" );
      return;
   }
   sprintf( query, pattern, table );
   insertstrings( query, pattern, '!', numofpairs, par   ); // properties
   insertstrings( query, pattern, '?', numofpairs, par+1 ); // values
   printf( "dbclient query: %s\n", query );

   // zur Datenbankl
   if( mysql_query( conn, query ) != 0 )
   {
      print_error( conn, "cannot insert", query );
   }
   else
   {
      unsigned long n;
      n = (unsigned long)mysql_affected_rows(conn);
      printf( "dbclient: %lu mysql_affected row%s\n", n, n == 1 ? "" : "s" );
   }
   free( query );
}


static void dump( MYSQL * conn )
{
   print_error( 0, "not implemented, yet", "-d" );
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
   case 't':
      if( argc < 5 )
         usage();
      if( argc % 2 != 1 )
         usage();
      createtable( conn, argv[2], (argc-3)/2, argv+3 );
      break;
   case 'x':
      if( argc != 3 )
         usage();
      deletetable( conn, argv[2] );
      break;
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
      if( argc > 2 )
         usage();
      dump(conn);
      break;
   default:
      usage();
   }


   disconnect(conn);
   exit(0);
}
