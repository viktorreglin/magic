#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bool.h"
#include "salloc.h"

#include "qdb.h"


static void usage( void )
{
   printf( "USAGE: qdb_dump DBName Table\n" );
   exit(1);
}


static void dump( char * db, char * table, QDB_TABLEENTRY * properties, int numofprop )
{
   char * query;
   int i, r, nfields;
   MYSQL * conn;
   MYSQL_RES * result;
   MYSQL_ROW   row;
   static char pattern[] = "select ! from %s;"; // %s = table, ! = properties

   conn = qdb_open( db );
   if( !conn )
      return;

   query = sql_buildquery( pattern, table, properties, numofprop, false );

   // zur Datenbank
   if( mysql_query( conn, query ) != 0 )
   {
      mysql_print_error( conn, "cannot dump", query );
   }
   else
   {
      result = mysql_store_result(conn);
      if( result )
      {
         nfields = mysql_num_fields(result);
         if( nfields != numofprop )
         {
            printf( "WARNING: %d properties but %d fields (should be equal)", numofprop, nfields );
         }
         r = 1;
         while( (row = mysql_fetch_row(result)) != NULL )
         {
            printf( "===== row %d:\n", r );
            for( i = 0; i < nfields; i++ )
            {
               printf( "   %s = '%s'\n", i < numofprop ? properties[i].name : "?", row[i] ? row[i] : "" );
            }
            r++;
         }
         mysql_free_result(result);
      }
      else
      {
         mysql_print_error( conn, "cannot store result", query );
      }
   }

   sfree( query );
   qdb_close( &conn );
}


int main( int argc, char * argv[] )
{
   char * db;
   char * table;
   QDB_TABLEENTRY * properties;
   QDB_TABLEENTRY * prop;
   int numofprop;
   static char proptypformat[] = "%-20s  %-10s%s\n";

   if( argc != 3 )
      usage();

   db    = argv[1];
   table = argv[2];

   properties = qdb_get_properties( db, table, &numofprop );
   if( !properties )
      exit(2);

   printf( "DATABASE %s  TABLE %s:  %d PROPERTIES\n", db, table, numofprop );
   if( numofprop <= 0 )
      exit(0);

   printf( proptypformat, "PROPERTY", "SQL TYPE", "" );
   prop = properties;
   while( prop->name )
   {
      printf( proptypformat, prop->name, prop->sqltype, prop->isstring ? "  (string)" : "" );
      prop++;
   }

   dump( db, table, properties, numofprop );

   sfree( properties );
   exit(0);
}

