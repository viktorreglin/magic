#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "port.h"
#include "bool.h"
#include "readfile.h"
#include "salloc.h"

#include "config.h"


typedef struct
{
   char * name;
   VTYPE  type;
   char * value;
   int intvalue;
} CONFIG_ENTRY;

typedef struct
{
   char * filecontent;
   int numentries;
   CONFIG_ENTRY * table;
} CONFIG_TABLE;



static char * charinline( char * str, char c )
/* ---------------------------------------------------------------- */
{
   while( *str )
   {
      if( *str == c )
         return str;
      if( *str == '\n' )
         return 0;
      str++;
   }
   return 0;
}


static bool isintnumber( char * s )
/* ---------------------------------------------------------------- */
{
   char * end;
   char * set;

   if( !s )
      return false;

   // skip leading blank
   while( (*s == ' ') || (*s == '\t') )
      s++;

   end = s + strlen(s);
   if( end == s )
      return false; // empty or blank string

   end--;
   // skip trailing blank
   while( (*end == ' ') || (*end == '\t') || (*end == '\r') )
      end--;

   if( *s == '0' )
   {
      s++;
      if( (*s == 'x') || (*s == 'X') )
      {
         set = "01234567890ABCDEFabcdef"; // hexadecimal
         s++;
      }
      else
         set = "01234567"; // oktal
   }
   else
   {
      set = "0123456789"; // decimal
   }
   for(;;)
   {
      if( !strchr( set, *s ) )
         return false;
      if( s == end )
         break;
      s++;
   }
   return true;
}


static int config_parse( char * filestr, char comment, CONFIG_ENTRY * table )
/* ---------------------------------------------------------------- */
/* liest das config"file" filestr und gibt Anzahl der Eintraege zurueck
   falls table != 0 werden table[0]..table[Anzahl-1] gefuellt
   ACHTUNG: wenn table != 0 ist, dann wird der in den Speicher geladene
   File-Inhalt (filestr) geaendert, d.h. man kann config_parse nicht 2mal mit
   table != 0 aufrufen ! */
{
int  lineno, idx;
bool incomment;
char * s;
char * nam;
char * namend;
char * val;
char * valend;
char * eqs;

   if( !filestr || !filestr[0] )
      return 0;

   idx = 0;
   incomment = false;
   lineno = 1;
   s = filestr;
   while( *s )
   {
      if( (*s == ' ') || (*s == '\t') || (*s == '\r') )
      {
         s++;
      }
      else if( *s == '\n' )
      {
         s++;
         incomment = false;
         lineno++;
      }
      else if( *s == comment )
      {
         s++;
         incomment = true;
      }
      else if( incomment )
      {
         s++;
      }
      else
      {
         // ok, hier sollte name=value kommen
         nam = s;
         eqs = charinline( nam, '=' );
         if( eqs )
         {
            val = eqs + 1;
            namend = eqs - 1;
            while( (*namend == ' ') || (*namend == '\t') )
               namend--;
            namend++;
            s = val;
            if( table )
            {
               *namend = 0;
               table[idx].name = nam;
               table[idx].type = is_unknown;
               while( (*val == ' ') || (*val == '\t') )
                  val++;
               if( *val == '"' )
               {                       // "...."
                  table[idx].type = is_string;
                  table[idx].intvalue = 0;
                  val++;
                  valend = charinline( val, '"' );
                  incomment = true;
               }
               else
               {  // vom ersten bis zum letzten nicht-blank-Zeichen
                  valend = strchr( val, '\n' );
                  valend--;
                  while( (*valend == ' ') || (*valend == '\t') || (*valend == '\r') )
                     valend--;
                  valend++;
                  if( *valend == '\n' )
                     lineno++;
                  else
                     incomment = true;
               }
               if( valend )
               {
                  *valend = 0;
                  s = valend+1;
               }
               table[idx].value = val;
               table[idx].intvalue = strtol( val, 0, 0 );
               if( table[idx].type == is_unknown )
               {
                  if( isintnumber(val) )
                     table[idx].type = is_int;
                  else
                     table[idx].type = is_string;
               }
            }
            else
            {
               incomment = true;
            }
            idx++;
         }
         else
         {
            fprintf(stderr, "ERROR in config file, line %d: no '='\n", lineno );
            incomment = true;
         }
      }
   }
   return idx;
}


CONFIG config_read( char * filename, char comment )
/* ================================================================ */
{
char * filestr;
CONFIG_TABLE * config_table;

   filestr = readfile( filename );
   if( !filestr )
      return 0;

   config_table = salloc( sizeof(config_table) );
   config_table->filecontent = filestr;
   config_table->numentries  = config_parse( filestr, comment, 0 );
   if( config_table->numentries > 0 )
   {
      config_table->table = salloc( config_table->numentries * sizeof(CONFIG_ENTRY) );
      config_parse( filestr, comment, config_table->table );
   }
   return config_table;
}


int config_num( CONFIG cfg )
/* ================================================================ */
{
   CONFIG_TABLE * config_table = cfg;
   if( !config_table )
      return 0;

   return config_table->numentries;
}


static int config_find( CONFIG_TABLE * config_table, char * name )
/* ---------------------------------------------------------------- */
{
int i;

   if( !config_table || !name || !name[0] )
      return -1;

   for( i = 0; i < config_table->numentries; i++ )
   {
      if( stricmp( name, config_table->table[i].name ) == 0 )
         return i;
   }
   return -1;
}


VTYPE config_type( CONFIG cfg, char * name )
/* ================================================================ */
{
int idx;

   CONFIG_TABLE * config_table = cfg;
   if( !config_table )
      return is_unknown;

   idx = config_find( config_table, name );
   if( idx < 0 )
      return is_unknown;

   return config_table->table[idx].type;
}


char * config_string( CONFIG cfg, char * name, char * def )
/* ================================================================ */
{
int idx;

   CONFIG_TABLE * config_table = cfg;
   if( !config_table )
      return def;

   idx = config_find( config_table, name );
   if( idx < 0 )
      return def;

   return config_table->table[idx].value;
}


int config_int( CONFIG cfg, char * name, int def )
/* ================================================================ */
{
int idx;

   CONFIG_TABLE * config_table = cfg;
   if( !config_table )
      return def;

   idx = config_find( config_table, name );
   if( idx < 0 )
      return def;

   return config_table->table[idx].intvalue;
}


#ifdef TEST_CONFIG

#include "test.h"

#define CONFIGFILE "testconfig.cfg"

int main( void )
{
FILE * f;
CONFIG cfg;

   f = fopen( CONFIGFILE, "w" );
   fprintf( f, "# comment\n" );
   fprintf( f, "aaa = QwertZ\n" );
   fprintf( f, "BBB = \"QwertZ \"\n" );
   fprintf( f, "ccc=string mit Leerzeichen\n" );
   fprintf( f, "ddd=4711\n" );
   fprintf( f, "eEe = 077\n" );
   fprintf( f, "fff = 0x6821\n" );
   fprintf( f, "ggg = 4711parfum\n" );
   fprintf( f, "hhh = \" 4711 \"\n" );
   fprintf( f, "iii = 08\n" );
   fprintf( f, "jjj = 0xG\n" );
   fprintf( f, "\n" );
   fprintf( f, "kkk = \"5 text und\" #Kommentar\n" );
   fclose(f);

   cfg = config_read( CONFIGFILE, '#' );
   TEST_EQ( config_num(cfg), 11);

   TEST_EQ( config_type( cfg, "AAA"), is_string);
   TEST_EQ( config_type( cfg, "bbb"), is_string);
   TEST_EQ( config_type( cfg, "ccc"), is_string);
   TEST_EQ( config_type( cfg, "DdD"), is_int);
   TEST_EQ( config_type( cfg, "eeE"), is_int);
   TEST_EQ( config_type( cfg, "FFF"), is_int);
   TEST_EQ( config_type( cfg, "GGG"), is_string);
   TEST_EQ( config_type( cfg, "HHH"), is_string);
   TEST_EQ( config_type( cfg, "iii"), is_string);
   TEST_EQ( config_type( cfg, "JJJ"), is_string);
   TEST_EQ( config_type( cfg, "kkk"), is_string);
   TEST_EQ( config_type( cfg, "aa") , is_unknown);

   TEST_EQ( config_int( cfg, "AAA", 7 ), 0);
   TEST_EQ( config_int( cfg, "bbb", 7 ), 0);
   TEST_EQ( config_int( cfg, "ccc", 7 ), 0);
   TEST_EQ( config_int( cfg, "DdD", 7 ), 4711);
   TEST_EQ( config_int( cfg, "eeE", 7 ), 077);
   TEST_EQ( config_int( cfg, "FFF", 7 ), 0x6821);
   TEST_EQ( config_int( cfg, "GGG", 7 ), 4711);
   TEST_EQ( config_int( cfg, "HHH", 7 ), 4711);
   TEST_EQ( config_int( cfg, "iii", 7 ), 0);
   TEST_EQ( config_int( cfg, "JJJ", 7 ), 0);
   TEST_EQ( config_int( cfg, "kkk", 7 ), 5);
   TEST_EQ( config_int( cfg, "aaaa", 7 ), 7);

   TEST_EQ_STR( config_string( cfg, "AAA", "ABC"), "QwertZ");
   TEST_EQ_STR( config_string( cfg, "bbb", "ABC"), "QwertZ ");
   TEST_EQ_STR( config_string( cfg, "ccc", "ABC"), "string mit Leerzeichen");
   TEST_EQ_STR( config_string( cfg, "DdD", "ABC"), "4711");
   TEST_EQ_STR( config_string( cfg, "eeE", "ABC"), "077");
   TEST_EQ_STR( config_string( cfg, "FFF", "ABC"), "0x6821");
   TEST_EQ_STR( config_string( cfg, "GGG", "ABC"), "4711parfum");
   TEST_EQ_STR( config_string( cfg, "HHH", "ABC"), " 4711 ");
   TEST_EQ_STR( config_string( cfg, "iii", "ABC"), "08");
   TEST_EQ_STR( config_string( cfg, "JJJ", "ABC"), "0xG");
   TEST_EQ_STR( config_string( cfg, "kkk", "ABC"), "5 text und");
   TEST_EQ_STR( config_string( cfg, "ZZZ", "ABC"), "ABC");

   printf(" %d errors\n", test_result );
   exit( test_result );
}
#endif
