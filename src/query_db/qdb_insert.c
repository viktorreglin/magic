#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bool.h"
#include "salloc.h"

#include "qdb.h"


static void usage( void )
{
   printf( "USAGE: qdb_insert [-p] DBName Table property value property value ...\n" );
   exit(1);
}


int main( int argc, char * argv[] )
{
   char * db;
   char * table;
   int i, numofpairs;
   QDB_ROW    tr;
   bool printquery = false;
   int idx = 1;

   if( argc < 5 )  // wenigstens ein property/value-Paar
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

   if( (argc-idx) & 1 )
      usage();   // keine halben Paare

   numofpairs = (argc-idx)/2;

   tr = qdb_begin_row( db, table );
   if(!tr)
   {
      printf("FATAL: table '%s' not found in database '%s'\n", table, db );
      exit(2);
   }
   for( i = 0; i < numofpairs; i++ )
   {
      if( !qdb_set_value( tr, argv[idx+2*i], argv[idx+1+2*i] ) )
         printf( "ERROR: property '%s' not found\n", argv[3+2*i] );
   }
   if( !qdb_end_row(tr, printquery) )
      printf( "ERROR: row NOT inserted in table '%s'\n", table );

   exit(0);
}
