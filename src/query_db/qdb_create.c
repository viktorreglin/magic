#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asprintf.h"
#include "bool.h"

#include "qdb.h"

#define MAXLINELENGTH 4000

typedef enum { key_none, key_primary, key_foreign } KEYTYPE;


static void usage( void )
{
   printf( "USAGE: qdb_create DBName TableDefinitionFile\n" );
   exit(1);
}


static void cantopen( char * fname )
{
   printf( "FATAL: cannot open '%s'\n", fname );
   exit(2);
}


char * trimline( char * line )
{
   char * beg;
   char * end;

   beg = line;
   while( (*beg == ' ') || (*beg == '\t') )
      beg++;

   end = line + strlen(line) - 1;
   while( (end >= beg) && ((*end == '\n') || (*end == '\r') || (*end == ' ') || (*end == '\t')) )
      end--;
   end++;
   *end = 0;

   return beg;
}


static int processline( int lineno, char * line, FILE * sqlfile, bool * firstelem )
// returns 0 iff ok, 1 iff error
{
   char * word;
   char * elem;
   char * mtyp;
   char * sqltyp;
   KEYTYPE kt;

   line = trimline(line);

   if( *line == 0 )
      return 0; // leere Zeile

   //printf( "line %d: %s\n", lineno, line );;;

   if( *line == '#' )
      return 0; // Kommentar

   word = strtok( line, " \t" );
   if( strcmp(word,"table") == 0 )
   {
      word = strtok( 0, " \t" );
      fprintf( sqlfile, "create table %s\n(", word );
      *firstelem = true;
   }
   else if( strcmp(word,"endtable") == 0 )
   {
      fprintf( sqlfile, "\n);\n\n" );
      *firstelem = false;
   }
   else
   {
      if( *word == '*' )
      {
         kt = key_primary;
         word++;
      }
      else if( *word == '+' )
      {
         kt = key_foreign;
         word++;
      }
      else
      {
         kt = key_none;
      }
      mtyp = strtok( 0, " \t" );
      if( !mtyp )
      {
         printf( "ERROR in line %d: type missing after %s", lineno, word );
         return 1;
      }
      sqltyp = sql_type(mtyp);
      if( !sqltyp )
      {
         printf( "ERROR in line %d: cannot determine sql type of %s", lineno, mtyp );
         return 1;
      }
      elem = word;
      fprintf( sqlfile, "%s\n  %-20s  %s", *firstelem ? "" : "," , elem, sqltyp );
      if( kt == key_primary )
      {
         fprintf( sqlfile, ",\n  primary key (%s)", elem );
      }
      else if( kt == key_foreign )
      {
         char * table2;
         char * elem2;
         word++;
         table2 = strtok( 0, " \t(" );
         if( !table2 )
         {
            printf( "ERROR in line %d: table name missing after %s", lineno, word );
            return 1;
         }
         elem2 = strtok( 0, " \t)" );
         if( !elem2 )
         {
            printf( "ERROR in line %d: element name missing after %s", lineno, table2 );
            return 1;
         }
         fprintf( sqlfile, "%s\n  foreign key (%s) references %s (%s)", *firstelem ? "" : ",", elem, table2, elem2 );
      }
      *firstelem = false;
   }
   return 0;
}


static int makesqlfile( char * tablefilename, char * sqlfilename )
// returns # of errors
{
   FILE * tablefile;
   FILE * sqlfile;
   bool firstelem;
   int lineno, errors;
   char line[MAXLINELENGTH+2];

   // printf( "tablefile=%s  sqlfile=%s\n", tablefilename, sqlfilename );;;

   tablefile = fopen( tablefilename, "r" );
   if( !tablefile )
      cantopen( tablefilename );

   sqlfile = fopen( sqlfilename, "w" );
   if( !sqlfile )
      cantopen( sqlfilename );

   lineno = 1;
   errors = 0;
   while( fgets( line, sizeof(line), tablefile) )
   {
      errors += processline( lineno, line, sqlfile, &firstelem );
      lineno++;
   }

   fclose( sqlfile );
   return errors;
}


static int makedb( char * name, char * sqlfilename )
{
   int rc;
   char cmd[1000];

   // erzeuge Datenbank
   sprintf( cmd, "mysql -e \"create database %s;\"", name );
   rc = system( cmd );
   if( rc != 0 )
      return rc;

   // erzeuge Tabellen
   sprintf( cmd, "mysql %s < %s", name, sqlfilename );
   rc = system( cmd );

   return rc;
}


int main( int argc, char * argv[] )
{
   char * dbname;
   char * tablefilename;
   char * sqlfilename;
   int rc;

   if( argc != 3 )
      usage();

   dbname        = argv[1];
   tablefilename = argv[2];
   sqlfilename   = asprintf( "%s.sql", dbname );

   rc = makesqlfile( tablefilename, sqlfilename );
   if( rc != 0 )
   {
      printf( "%d errors in %s, no database created\n", rc, tablefilename );
      exit(3);
   }

   rc = makedb( dbname, sqlfilename );
   if( rc != 0 )
   {
      printf( "no database created, rc=%d\n", rc );
      exit(4);
   }

   exit(0);
}

