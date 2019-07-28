#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qdb.h"


static void usage( void )
{
   printf( "USAGE: qdb_query [-p] DBName Table Query\n" );
   exit(1);
}


static void printheader( QDB_RESULTROW head )
{
   int i;
   for( i = 0; i < head.numofvalues; i++ )
      printf( "(%d)%s  ", i+1, head.value[i].strval );
   printf("\n");
}


static void printvalues( QDB_RESULTROW head, QDB_RESULTROW row )
{
   int i;
   for( i = 0; i < row.numofvalues; i++ )
   {
      if( head.value[i].intval & (QDB_TYPE_IS_STRING|QDB_TYPE_IS_DATE) )
         printf( "(%d)'%s' ", i+1, row.value[i].strval );
      else
         printf( "(%d)%d ", i+1, row.value[i].intval );
   }
   printf("\n");
}


int main( int argc, char * argv[] )
{
   char * db;
   char * table;
   char * query;
   QDB_RESULT * pres;
   int nrows, i;
   bool printquery = false;
   int idx = 1;

   if( argc < 1 )
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

   printf( "DB=%s\nTable=%s\nQuery='%s'\n", db, table, query );
   pres = qdb_query( db, table, &nrows, query, printquery );
   if( nrows < 0 )
      printf( "error, no result\n" );
   else if ( nrows == 0 )
      printf( "empty result\n" );
   else
   {
      printf( "%d entr%s found:\n\n", nrows, nrows==1 ? "y" : "ies" );
      printheader( pres->row[0] );
      for( i = 1; i <= nrows; i++ )
      {
         printf( "\n#%d:  ", i );
         printvalues( pres->row[0], pres->row[i] );
      }
   }
   if( pres )
      qdb_free(pres);
   printf("\n");
   exit(0);
}

