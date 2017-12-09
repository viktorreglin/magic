#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bool.h"
#include "salloc.h"

#include "qdb.h"


static void usage( void )
{
   printf( "USAGE: qdb_insert DBName Table property value property value ...\n" );
   exit(1);
}


int main( int argc, char * argv[] )
{
   char * db;
   char * table;
   int i, numofpairs;
   QDB_ROW    tr;

   if( (argc < 5) || ((argc & 1) == 0) )  // wenigestens ein property/value-Paar, kein halbes Paar
      usage();

   numofpairs = (argc-3)/2;

   db    = argv[1];
   table = argv[2];

   tr = qdb_begin_row( db, table );
   if(!tr)
   {
      printf("FATAL: table '%s' not found in database '%s'\n", table, db );
      exit(2);
   }
   for( i = 0; i < numofpairs; i++ )
   {
      if( !qdb_set_value( tr, argv[3+2*i], argv[4+2*i] ) )
         printf( "ERROR: property '%s' not found\n", argv[3+2*i] );
   }
   if( !qdb_end_row(tr) )
      printf( "ERROR: row NOT inserted in table '%s'\n", table );

   exit(0);
}
