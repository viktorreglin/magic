#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asprintf.h"
#include "bool.h"
//#include "macros.h"
//#include "port.h"
//#include "salloc.h"
//#include "str.h"

#include "qdb.h"

#define MAXLINELENGTH 4000

typedef enum { key_none, key_primary, key_foreign } KEYTYPE;


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


static void processline( int lineno, char * line, FILE * sqlfile, bool * firstelem )
{
   char * word;
   char * mtyp;
   char * sqltyp;
   KEYTYPE kt;

   line = trimline(line);

   if( *line == 0 )
      return; // leere Zeile

   //printf( "line %d: %s\n", lineno, line );;;

   if( *line == '#' )
      return; // Kommentar

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
         char * primtable;
         char * elemname;
         kt = key_foreign;
         word++;
         primtable = strtok( 0, " \t(" );
         if( !primtable )
         {
            printf( "ERROR in line %d: table name missing after %s", lineno, word );
            return;
         }
         elemname = strtok( 0, " \t)" );
         if( !elemname )
         {
            printf( "ERROR in line %d: element name missing after %s", lineno, primtable );
            return;
         }
         fprintf( sqlfile, "%s\n  constraint _%-8s  foreign key key (%s) references %s (%s)", *firstelem ? "" : ",", word, elemname, primtable, elemname );
         *firstelem = false;
         return;
      }
      else
      {
         kt = key_none;
      }
      mtyp = strtok( 0, " \t" );
      if( !mtyp )
      {
         printf( "ERROR in line %d: type missing after %s", lineno, word );
         return;
      }
      sqltyp = sql_type(mtyp);
      if( !sqltyp )
      {
         printf( "ERROR in line %d: cannot determine sql type of %s", lineno, mtyp );
         return;
      }
      fprintf( sqlfile, "%s\n  %-20s  %s", *firstelem ? "" : "," , word, sqltyp );
      if( kt == key_primary )
         fprintf( sqlfile, ",\n  constraint _%-8s  pimary key (%s)", word, word );
      *firstelem = false;
   }
}


static void makesqlfile( char * tablefilename, char * sqlfilename )
{
   FILE * tablefile;
   FILE * sqlfile;
   bool firstelem;
   int lineno;
   char line[MAXLINELENGTH+2];

   printf( "tablefile=%s  sqlfile=%s\n", tablefilename, sqlfilename );;;

   tablefile = fopen( tablefilename, "r" );
   if( !tablefile )
      cantopen( tablefilename );

   sqlfile = fopen( sqlfilename, "w" );
   if( !sqlfile )
      cantopen( sqlfilename );

   lineno = 1;
   while( fgets( line, sizeof(line), tablefile) )
   {
      processline( lineno, line, sqlfile, &firstelem );
      lineno++;
   }

   fclose( sqlfile );
}


static int makedb( char * name, char * sqlfilename )
{
   printf( "dbname=%s  sqlfile=%s\n", name, sqlfilename );;;

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
   sqlfilename   = asprintf( "%s.sql", tablefilename );

   makesqlfile( tablefilename, sqlfilename );
   rc = makedb( argv[1], sqlfilename );

   return rc;
}

