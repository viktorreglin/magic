#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qdb.h"


static void usage( void )
{
   printf( "USAGE: qdb_erase [-p] DBName Table Query\n" );
   exit(1);
}


int main( int argc, char * argv[] )
{
   char * db;
   char * table;
   char * query;
   int nrows;
   bool printquery = false;
   int idx = 1;

   if( argc < 2 )
      usage();

   if( argv[1][0] == '-' )
   {
      if( strcmp(argv[1],"-p") == 0 )
         printquery = true;
      else
         usage();
      idx = 2;
   }
   if( argc != (idx+3) )
      usage();

   db    = argv[idx++];
   table = argv[idx++];
   query = argv[idx++];

   // printf( "DB=%s\nTable=%s\nQuery='%s'\n", db, table, query );;;
   nrows = qdb_erase( db, table, query, printquery );
   if( nrows < 0 )
   {
      printf( "ERROR, nothing changed\n" );
      exit(3);
   }
   else if( nrows == 0 )
      printf( "no entries found, nothing changed\n" );
   else
      printf( "%d row%s erased.", nrows, nrows > 1 ? "s" : "" );

   exit(0);
}

