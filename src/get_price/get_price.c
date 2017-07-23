// Downloads and unpacks the cards database.

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define FILELOCATION "../../data/cardprice.html"

void get_url( char * card, char * edition, char * url ){
   
   char * url_beginning = "https://www.magickartenmarkt.de/Products/Singles/";
   sprintf( url, "%s%s%s%s", url_beginning, edition, "/", card );
   
}


void download_page( FILE * fp, char * url ){
   CURL * curl;
   CURLcode res;
   char price[8];
   
   char file_name[FILENAME_MAX] = FILELOCATION;
   
   curl = curl_easy_init();
   if ( curl )
   {
      fp = fopen( file_name,"wb" );
      curl_easy_setopt( curl, CURLOPT_URL, url );
      curl_easy_setopt( curl, CURLOPT_USERAGENT, "curl/7.39.0" );
      curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, NULL );
      curl_easy_setopt( curl, CURLOPT_WRITEDATA, fp );
      res = curl_easy_perform( curl );
      curl_easy_cleanup( curl );
            
      fclose( fp );
   }
}


static long filelength( FILE * fp )
{
   long ende;

   fseek( fp, 0L, SEEK_END );
   ende = ftell( fp );
   rewind( fp );
   return ende;
}


void price_string( FILE * fp, char * price ){
   char * string_before_price = "Preistendenz:";//"Preistendenz:</td><td class=\"outerBottom outerRight col_Odd col_1 cell_2_1\">";//
   char * string_after_price = " &#x20AC;</td>";//" &#x20AC;</td></tr></tbody></table></div></div></div></div></div><div class=\"MKMBlockFooter brad0033 skmgrad\">";
   int char_delta = 76;
   char * start;
   char * end;
   int len;
   char * fp_string;
   long filelen;
   
   filelen = filelength( fp );
   
   fp_string = malloc( filelen + 1 );
   fread( fp_string, filelen, 1, fp );
   fp_string[ filelen ] = 0;
   
   start = strstr( fp_string, string_before_price );
   end = strstr( fp_string, string_after_price );
   len = end - start;
   printf( "len: %d\n", len  );
   
   sprintf( price, "%.*s", len-char_delta, start+char_delta );
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


int main( void ){
   char * card = "Price+of+Progress";
   char * edition = "Exodus";
   char url[256];
   char price[1000];
   float cardprice;
   FILE * fp;
   
   get_url( card, edition, url );
   
   download_page( fp, url );
   
   fp = fopen( FILELOCATION, "r" );
   price_string( fp, price );
   fclose ( fp );
   
   cardprice = price_to_number( price );
   
   printf( "%f\n", cardprice );
   
   return 0;
}