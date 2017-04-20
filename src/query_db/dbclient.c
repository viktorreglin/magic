// db client

#include <stdlib.h>
#include <stdio.h>
#include <mysql.h>

static char * s_host    = NULL;
static char * s_user    = "root";
static char * s_passwd  = NULL;
static char * s_dbname  = "magic1";
static char * s_socket  = NULL;
static unsigned int s_port = 0;

static MYSQL * s_conn;


static print_error( MYSQL * conn, char * txt1, char * txt2 )
{
   fprintf( stderr, "ERROR: %s %s\n", txt1, txt2 );
   if( conn )
      fprintf( stderr, "mysql: %s\n", mysql_error(conn) );
}


int main( int argc, char * argv[] )
{
   if( mysql_library_init( 0, NULL, NULL) )
   {
      print_error( NULL, "cannot initialize mysql library", "" );
      exit(1);
   }
   s_conn = mysql_init( NULL );
   if( !s_conn )
   {
      print_error( NULL, "cannot initialize mysql connection handler", "" );
      exit(2);
   }
   if( !mysql_real_connect( s_conn, s_host, s_user, s_passwd, s_dbname, s_port, s_socket, 0 ) )
   {
      print_error( s_conn, "cannot connect to mysql db", s_dbname );
      mysql_close( s_conn );
      exit(3);
   }

   mysql_close( s_conn );
   mysql_library_end();
   exit(0);
}
