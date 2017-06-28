#include <stdio.h>
#include <ctype.h> // Für tolower().

#include "str.h"


// Vergleicht die Länge von zwei STR Variablen.
int compareLength( STR str1, STR str2 ){
   if( str1.len == str2.len ){
      return 1;  
   }
   return 0;
}


// Vergleicht nacheinander die Zeichen von zwei STR Variablen.
int compareChars( STR str1, STR str2 ){
   int i;
      
   for( i = 0; i < str1.len; i++ ){
      if( !( tolower( str1.ptr[i] ) == tolower( str2.ptr[i] ) ) ){
         return 0;  
      }
   }
   return 1;
}


// Fügt comareLength und compareChars zusammen.
int compareStrings( STR str1, STR str2 ){
   if( compareLength( str1, str2 ) && compareChars( str1, str2 ) ){
      return 1;
   }
   return 0;
}