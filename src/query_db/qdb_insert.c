#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bool.h"
#include "salloc.h"

#include "qdb.h"

#undef API

static void usage( void )
{
   printf( "USAGE: qdb_insert DBName Table property value property value ...\n" );
   exit(1);
}

#ifndef API
int fillvalue( QDB_TABLEENTRY * properties, char * name, char * value )
// trage name/value in die Tabelle ein, return: 1 if eingetragen, sonst 0
{
   QDB_TABLEENTRY * prop;

   prop = properties;
   while( prop->name )
   {
      if( strcmp( prop->name, name ) == 0 )
      {
         if( prop->value )
         {
            printf( "WARNIMG: property %s has already a value (%s), ignoring new value(%s)\n", name, prop->value, value );
            return 0;
         }
         prop->value = value;
         return 1;
      }
      prop++;
   }
   printf( "ERROR: property %s not found\n", name );
   return 0;
}


static void insert( char * db, char * table, QDB_TABLEENTRY * properties, int numofprop )
{
   char * query;
   MYSQL * conn;
   static char pattern[] = "insert into %s (!) values (?);"; // %s = table, ! = properties, ? = values

   conn = sql_open( db );
   if( !conn )
      return;

   query = sql_buildquery( pattern, table, properties, numofprop, true );

   // zur Datenbank
   if( mysql_query( conn, query ) != 0 )
   {
      sql_print_error( conn, "cannot: ", query );
   }
   else
   {
      unsigned long n;
      n = (unsigned long)mysql_affected_rows(conn);
      printf( "%lu affected row%s\n", n, n == 1 ? "" : "s" );
   }

   sfree( query );
   sql_close( conn );
}
#endif

int main( int argc, char * argv[] )
{
   char * db;
   char * table;
   int i, numofpairs;
#ifdef API
   QDB_ROW    tr;
#else
   QDB_TABLEENTRY * properties;
   int numofprop, numofval;
#endif

   if( (argc < 5) || ((argc & 1) == 0) )  // wenigestens ein property/value-Paar, kein halbes Paar
      usage();

   numofpairs = (argc-3)/2;

   db    = argv[1];
   table = argv[2];

#ifdef API
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
#else
   // hole Property-Tabelle aus der DB
   properties = qdb_get_properties( db, table, &numofprop );
   if( !properties )
      exit(2);

   printf( "%d PROPERTIES\n", numofprop );
   if( numofprop <= 0 )
      exit(2);

   // trage die zu setzenden Werte ein
   numofval = 0;
   for( i = 0; i < numofpairs; i++ )
      numofval += fillvalue( properties, argv[3+2*i], argv[4+2*i] );

   if( numofval != numofpairs )
      printf( "WARNING: %d property/value pairs given, but only %d are valid\n", numofpairs, numofval );

   printf( "got %d value%s\n", numofval, numofval==1 ? "" : "s" );
   if( numofval < numofprop )
      printf( "WARNIMG: will fill %d properties with empty value\n", numofprop-numofval );

   insert( db, table, properties, numofprop );

   sfree( properties );
#endif
   exit(0);
}
