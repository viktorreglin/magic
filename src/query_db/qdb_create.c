#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asprintf.h"
//#include "bool.h"
//#include "macros.h"
//#include "port.h"
//#include "salloc.h"
//#include "str.h"

#include "qdb.h"



static void usage( void )
{
   printf( "USAGE: qdb_create Name TableDefinitionFile\n" );
   exit( 1 );
}


static void cantopen( char * fname )
{
   printf( "FATAL: cannot open '%s'\n", fname );
   exit( 2 );
}


static void makesqlfile( char * tablefilename, char * sqlfilename )
{
   FILE * tablefile;
   FILE * sqlfile;

   tablefile = fopen( tablefilename, "r" );
   if( !tablefile )
      cantopen( tablefilename );

   sqlfile = fopen( sqlfilename, "w" );
   if( !sqlfile )
      cantopen( sqlfilename );


   fclose( sqlfile );
}


static int makedb( char * name, char * sqlfilename )
{
   return 0;
}


int main( int argc, char * argv[] )
{
   char * tablefilename;
   char * sqlfilename;
   int rc;

   if( argc != 3 )
      usage();

   tablefilename = argv[2];
   sqlfilename   = asprintf( "%.s.sql", tablefilename );

   makesqlfile( tablefilename, sqlfilename );
   rc = makedb( argv[1], sqlfilename );

   return rc;
}
