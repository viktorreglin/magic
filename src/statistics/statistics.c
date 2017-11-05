// Berechnet Statistiken zu einem Deck.


#include <stdio.h>
#include <string.h>

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


// Gibt von einer Karte die Manakosten zurück.
void manacosts( char * cardcosts, int * occurences ){
   char colors[6] = {'B', 'C', 'G', 'U', 'R', 'W'};
   int i;
   
   for( i = 0; i < ( sizeof( colors ) / sizeof( char ) ); i++ ){
      occurences[i] += occurence( colors[i], cardcosts );
   }
}


// Gibt das n-te Symbol einer Datei wieder.
char char_n( FILE * fp, int n, char * buf ){
   fgets( buf, STRINGSIZE, fp );
   
   return buf[n];
}


// Findet in einer Datei die erste Zeile, die mit dem angegebenen Zeichen beginnt. Gibt die Zeile as String zurück.
long row_num( FILE * fp, char symbol, char * buf, long pos_in_file ){
   
   fseek( fp, pos_in_file, SEEK_SET ); // Starte bei der letzten Position, nicht am Anfang.
   
   for(;;){
      if( fgets( buf, STRINGSIZE, fp ) == NULL || buf[0] == symbol ) break;
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
   
   pos = string_pos( search_for, search_in );
   strncpy( buf, search_in, pos );
   return occurence( delimiter, buf );
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
   end = nth_pos( search_in, delimiter, n + 1 ) - 1;
   
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



// Main.
void main(){
   const int del_pos = 1; // Trennzeichen ist das zweite Zeichen in einer Datei.
   char header_row = 'H'; // Überschriften stehen in der Zeile, die mit H beginnt.
   char card_row = 'C'; // Kartenzeilen beginnen mit C.
   char * attribute = "manaCost";
   
   FILE * fp;
   char buf[STRINGSIZE];
   char substring[STRINGSIZE];
   char delimiter;
   int del_count;
   int occurences[6] = {0,0,0,0,0,0}; // 6 Farben, inklusive "farblos".
   int i;
   long pos_in_file = 0;
   long length;
   
   fp = fopen( FILENAME, "r" );
   delimiter = char_n( fp, del_pos, buf ); // Bestimmt das Trennzeichen.
   row_num( fp, header_row, buf, 0 ); // Packt die H-Zeile in ein Array.
   del_count = nth_delimiter( attribute, buf, delimiter ); //Gibt an, das wievielte Element manaCost ist.
   
   length = filelength( fp );
   
   for(;;){
      if( pos_in_file == length ) break; // Aufhören sobald das Ende der Datei erreicht wurde.
      pos_in_file = row_num( fp, card_row, buf, pos_in_file ); // Packt die erste T-Zeile in ein Array.
      nth_element( buf, delimiter, del_count, substring ); // Packt den Teilstring der Manakosten in ein Array.
      
      manacosts( substring, occurences ); // Zählt die Manakosten pro Farbe.
   }
   
   printf( "\nManacosts:\n" );
   for( i = 0; i < ( sizeof( occurences ) / sizeof( int ) ); i++ ){
      printf( " %d ", occurences[i] );
   }
   printf( "\n" );
   
   fclose( fp );
   
}