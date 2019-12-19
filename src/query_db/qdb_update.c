#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qdb.h"


static void usage( void )
{
   printf( "USAGE: qdb_update [-p] DBName Table Query property value property value ...\n" );
   exit(1);
}


int main( int argc, char * argv[] )
{
   char * db;
   char * table;
   char * query;
   int nrows, numofpairs;
   bool printquery = false;
   int idx = 1;

   if( argc < 6 )
      usage();

   if( argv[1][0] == '-' )
   {
      if( strcmp(argv[1],"-p") == 0 )
         printquery = true;
      else
         usage();
      idx = 2;
   }

   db    = argv[idx++];
   table = argv[idx++];
   query = argv[idx++];

   if( (argc-idx) & 1 )
      usage();   // keine halben Paare

   numofpairs = (argc-idx)/2;

   printf( "numofpairs = %d\n", numofpairs );
   exit(0);
}

