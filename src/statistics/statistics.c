// Berechnet Statistiken zu einem Deck.

/* HINZUFÜGEN
 * Header-Datei mit enum für die Farben?
 * Zählen der Kartentypen
 * Zählen der Raritäten: Karten nacheinander durchgehen. Es gibt die Möglichkeiten Common, Uncommon, Rare, Mythic Rare, Basic Land, Special. Spaltenüberschrift ist "rarity".
 * 
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define STRINGSIZE 10000
#define FILENAME "../../test/sample.mc"



// Gibt an, wie häufig ein Zeichen in einem String vorkommt.
int occurence( char search_for, char * search_in ){
   int i, count = 0;
   
   for( i = 0; i < strlen( search_in ); i++ ){
      if( search_in[i] == search_for ) count++;
   }
   
   return count;
}


// Gibt von einer Karte die Anzahlen der Manasymbole zurück.
void manacosts( char * cardcosts, int * symbol_counts ){
   char colors[6] = {'B', 'C', 'G', 'U', 'R', 'W'};
   int i;
   
   for( i = 0; i < ( sizeof( colors ) / sizeof( char ) ); i++ ){
      symbol_counts[i] += occurence( colors[i], cardcosts );
   }
}


// Gibt von einer Karte die Rarität zurück.
void rarity( char * rarity_string, int * rarity_counts ){
   char *rarities[6] = { "Common", "Uncommon", "Rare", "Mythic Rare", "Special", "Basic Land" };
   int i;
   
   for( i = 0; i < ( sizeof( rarities ) / sizeof( rarities[0] ) ); i++ ){
      if( strstr( rarity_string, rarities[i] ) && rarity_string[0] == rarities[i][0] ){
         rarity_counts[i]++;
         break;
      }
   }
}


// Gibt von einer Karte die Typen zurück.
void type( char * types_string, int * types_counts ){
   char *types[7] = { "Artifact", "Creature", "Enchantment", "Instant", "Land", "Sorcery", "Planeswalker" };
   int i;
   
   for( i = 0; i < ( sizeof( types ) / sizeof( types[0] ) ); i++ ){
      if( strstr( types_string, types[i] ) ){
         types_counts[i]++;
      }
   }
}


// Gibt das n-te Symbol einer Datei wieder.
char char_n( FILE * fp, int n, char * buf ){
   fgets( buf, STRINGSIZE, fp );
   
   return buf[n];
}


// Findet in einer Datei die nächste Zeile, die mit dem angegebenen Zeichen beginnt. Gibt die Zeile as String zurück.
long row_num( FILE * fp, char symbol, char * buf, long pos_in_file ){
   
   fseek( fp, pos_in_file, SEEK_SET ); // Starte bei der letzten Position, nicht am Anfang.
   
   for(;;){
      if( fgets( buf, STRINGSIZE, fp ) == NULL ) {
         memset( buf, 0, STRINGSIZE ); // Setzt alle Elemente von buf auf "0". Damit nicht die letzte Zeile des Decks ausgewertet wird, egal ob sie mit C anfängt.
         break;
      }
      if( buf[0] == symbol ) break;
   }
   
   return ftell( fp ); // Position in der Datei merken.
}


// Gibt die Position eines angegebenen Teilstrings in einem String zurück.
int string_pos( char * search_for, char * search_in ){
   return strstr( search_in, search_for ) - search_in;
}


// Gibt zurück, nach wie vielen Trennzeichen der angegebene Teilstring in einem String kommt.
int nth_delimiter( char * search_for, char * search_in, char delimiter ){
   char buf[STRINGSIZE];
   int pos;
   
   // search_for muss mit "|" anfangen, da sonst die Suche nach "types" auch bei "subtypes" Erfolg hätte.
   long len = strlen( search_for );
   int i;
   char *search_for_pipe = malloc( len + 1 + 1 );
   search_for_pipe[0] = delimiter;
   for( i = 1; i < len + 1; i++ ){
      search_for_pipe[i] = search_for[i-1];
   }
   search_for_pipe[i + 1] = '\0';
   
   // Eigentlicher Code. MUSS NOCH WIEDER EINS ABZIEHEN?
   pos = string_pos( search_for_pipe, search_in );
   strncpy( buf, search_in, pos );
   free(search_for_pipe);
   return occurence( delimiter, buf ) + 1;
}


// Gibt die Position zurück, an der das angegebene Zeichen n Mal im gegebenen String vorgekommen ist.
int nth_pos( char * search_in, char delimiter, int n ){
   int i, count = 0;
   
   for( i = 0; i < strlen( search_in ); i++ ){
      if( search_in[i] == delimiter ){
         count++;
         if( count == n ) break;
      }
   }
   
   return i;
}


// Nimmt einen String, ein Trennzeichen und eine Anzahl (n) an Trennzeichen als Argumente. Gibt den Teilstring zwischen dem angegebenen n-ten Trennzeichen und dem darauffolgenden Trennzeichen zurück.
void nth_element( char * search_in, char delimiter, int n, char * substring ){
   int start, end;
   
   start = nth_pos( search_in, delimiter, n ) + 1;
   end = nth_pos( search_in, delimiter, n + 1 );
   
   sprintf( substring, "%.*s", end - start, search_in + start );
}


// Gibt die Länge einer Datei zurück.
static long filelength( FILE * fp )
{
   long ende;

   fseek( fp, 0L, SEEK_END );
   ende = ftell( fp );
   rewind( fp );
   return ende;
}


// Füllt einen Array mit den Anzahlen der Manasymbole eines Decks.
void manasymbols( FILE * fp, long filelength, int * symbol_counts, char card_row, char * buf, char * substring, char delimiter, int del_count ){
   long  pos_in_file = 0;
   int   i;

   for(;;){
      if( pos_in_file == filelength ) break; // Aufhören sobald das Ende der Datei erreicht wurde.
      pos_in_file = row_num( fp, card_row, buf, pos_in_file ); // Packt die nächste C-Zeile in ein Array.
      nth_element( buf, delimiter, del_count, substring ); // Packt den Teilstring der Manakosten in ein Array.
      
      manacosts( substring, symbol_counts ); // Zählt die Manasymbole pro Farbe.
   }
}


// Füllt einen Array mit den Anzahlen der Kartenraritäten eines Decks.
void rarities( FILE * fp, long filelength, int * rarity_counts, char card_row, char * buf, char * substring, char delimiter, int del_count ){
   long  pos_in_file = 0;
   int   i;

   for(;;){
      if( pos_in_file == filelength ) break; // Aufhören sobald das Ende der Datei erreicht wurde.
      pos_in_file = row_num( fp, card_row, buf, pos_in_file ); // Packt die nächste C-Zeile in ein Array.
      nth_element( buf, delimiter, del_count, substring ); // Packt den Teilstring der Rarität in ein Array.
      
      rarity( substring, rarity_counts ); // Gibt die Rarität der Karte zurück.
   }
}


// Füllt einen Array mit den Anzahlen der Kartentypen eines Decks.
void types( FILE * fp, long filelength, int * types_counts, char card_row, char * buf, char * substring, char delimiter, int del_count ){
   long  pos_in_file = 0;
   int   i;

   for(;;){
      if( pos_in_file == filelength ) break; // Aufhören sobald das Ende der Datei erreicht wurde.
      pos_in_file = row_num( fp, card_row, buf, pos_in_file ); // Packt die nächste C-Zeile in ein Array.
      nth_element( buf, delimiter, del_count, substring ); // Packt den Teilstring der Rarität in ein Array.
      
      type( substring, types_counts ); // Gibt die Typen der Karte zurück.
   }
}


// Main.
void main(){
   int   del_pos = 1; // Trennzeichen ist das zweite Zeichen in einer Datei.
   int   del_count;
   int   symbol_counts[6] = { 0 }; // 6 Farben, inklusive "farblos".
   int   rarity_counts[6] = { 0 }; // 6 Raritäten: "Common", "Uncommon", "Rare", "Mythic Rare", "Special", "Basic Land"
   int   types_counts[7] = { 0 }; // 7 Typen: "Artifact", "Creature", "Enchantment", "Instant", "Land", "Sorcery", "Planeswalker"
   int   i;
   
   char  header_row = 'H'; // Überschriften stehen in der Zeile, die mit H beginnt.
   char  card_row = 'C'; // Kartenzeilen beginnen mit C.
   char  buf[STRINGSIZE];
   char  substring[STRINGSIZE];
   char  delimiter;
   char  *mana_heading = "manaCost";
   char  *rarity_heading = "rarity";
   char  *types_heading = "types";
   
   FILE  *fp;
   long  length;
    
   fp = fopen( FILENAME, "r" );
   length = filelength( fp );
   delimiter = char_n( fp, del_pos, buf ); // Bestimmt das Trennzeichen.
   row_num( fp, header_row, buf, 0 ); // Packt die H-Zeile in ein Array.
   
   
   // Manasymbole:   
   del_count = nth_delimiter( mana_heading, buf, delimiter ); //Gibt an, das wievielte Element manaCost ist.
   manasymbols( fp, length, symbol_counts, card_row, buf, substring, delimiter, del_count );
   
   printf( "\nManasymbols:\n" );
   for( i = 0; i < ( sizeof( symbol_counts ) / sizeof( int ) ); i++ ){
      printf( " %d ", symbol_counts[i] );
   }
   printf( "\n" );
   
   
   // Raritäten:
   row_num( fp, header_row, buf, 0 ); // Setzt buf zurück auf die H-Zeile.
   del_count = nth_delimiter( rarity_heading, buf, delimiter ); //Gibt an, das wievielte Element rarity ist.
   rarities( fp, length, rarity_counts, card_row, buf, substring, delimiter, del_count );
   
   printf( "\nRarities:\n" );
   for( i = 0; i < ( sizeof( rarity_counts ) / sizeof( int ) ); i++ ){
      printf( " %d ", rarity_counts[i] );
   }
   printf( "\n" );
   
   
   // Typen:
   row_num( fp, header_row, buf, 0 ); // Setzt buf zurück auf die H-Zeile.
   del_count = nth_delimiter( types_heading, buf, delimiter ); //Gibt an, das wievielte Element types ist.
   types( fp, length, types_counts, card_row, buf, substring, delimiter, del_count );
   
   printf( "\nTypes:\n" );
   for( i = 0; i < ( sizeof( types_counts ) / sizeof( int ) ); i++ ){
      printf( " %d ", types_counts[i] );
   }
   printf( "\n" );
   
   
   fclose( fp );
}