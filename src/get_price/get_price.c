// Preissssss

#include "get_price.h"

void get_url( char * card, char * edition, char * url ){

   char * url_beginning = "https://www.cardmarket.com/en/Magic/Products/Singles/";
   sprintf( url, "%s%s%s%s", url_beginning, edition, "/", card );
   //printf( "\nURL: %s\n", url );

}


int download_page( FILE * fp, char * url, long max_seconds ){
   CURL * curl;
   CURLcode res;
   static int found = -1;

   char file_name[FILENAME_MAX] = FILELOCATION;

   unlink(file_name);
   curl = curl_easy_init();
   if ( curl ){
      fp = fopen( file_name,"wb" );
      curl_easy_setopt( curl, CURLOPT_URL, url );
      curl_easy_setopt( curl, CURLOPT_USERAGENT, "curl/7.39.0" );
      curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, NULL );
      curl_easy_setopt( curl, CURLOPT_WRITEDATA, fp );
      curl_easy_setopt( curl, CURLOPT_TIMEOUT, max_seconds );
      res = curl_easy_perform( curl );
      curl_easy_cleanup( curl );

      found = res;
      fclose( fp );
   }

   return found;
}


long filelength( FILE * fp )
{
   long ende;

   fseek( fp, 0L, SEEK_END );
   ende = ftell( fp );
   rewind( fp );
   return ende;
}


int price_string( FILE * fp, char * price ){
   char * string_before_price = "Price Trend</dt>";
   //char * string_after_price = " &#x20AC;</span>"; //LOOKS TO BE DIFFERENT NOW ON 2020-02-12
   char * string_after_price = "</span></dd><dt class=\"col-6 col-xl-5\">30-days average price";
   int char_delta = 49; // Anzahl Zeichen von Beginn string_before_price bis direkt vor dem wirklichen Preis
   char * start;
   char * end;
   int len;
   char * fp_string;
   long filelen;

   filelen = filelength( fp );
   if( filelen <= 0 )
      return 0;

   fp_string = malloc( filelen + 1 );
   fread( fp_string, filelen, 1, fp );
   fp_string[ filelen ] = 0;

   start = strstr( fp_string, string_before_price );
   end = strstr( fp_string, string_after_price );
   len = end - start;

   if( len ){
      sprintf( price, "%.*s", len-char_delta, start+char_delta );
      return 1;
   } else return 0;
}


char * replace_char( char * str, char find, char replace ){
    char * current_pos = strchr( str,find );
    while( current_pos ){
        * current_pos = replace;
        current_pos = strchr( current_pos,find );
    }
    return str;
}


float price_to_number( char * price_string ){
   char * new_string;

   new_string = replace_char( price_string, ',', '.' );
   return atof( new_string );
}


char * format_name( char * name ){
   return replace_char( name, ' ', '+' );
}


float get_price( char * card, char * edition, char * url, char * price, long max_seconds ){
   float cardprice;
   FILE * fp = 0;
   int got_price = 0;
   int res;

   get_url( card, edition, url );

   res = download_page( fp, url, max_seconds );
   if( res == 0 ){
      fp = fopen( FILELOCATION, "r" );
      got_price = price_string( fp, price );
      fclose ( fp );
      if( !got_price ){
         printf( "(no price found)" );
         return 0.0;
      }
   }
   else
      printf( "(curl error %d)", res );

   if( got_price ){
         cardprice = price_to_number( price );
         if( cardprice < 0.01 )
            printf("(card price 0? [%s])", price );
         return cardprice;
      }

   printf( "(time out)" );
   return 0.0;
}
