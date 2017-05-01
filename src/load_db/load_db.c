// Downloads and unpacks the cards database.

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <zip.h>

#define FILESIZE 30000000


// Lade die Kartendatebank herunter.
void download(){
   CURL * curl;
   FILE * fp;
   CURLcode res;
   char * url = "https://mtgjson.com/json/AllSets.json.zip";
   char file_name[FILENAME_MAX] = "../../data/AllSets.json.zip";
   curl = curl_easy_init();
   if ( curl )
   {
      fp = fopen( file_name,"wb" );
      curl_easy_setopt( curl, CURLOPT_URL, url );
      curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, NULL );
      curl_easy_setopt( curl, CURLOPT_WRITEDATA, fp );
      res = curl_easy_perform( curl );
      curl_easy_cleanup( curl );
      fclose( fp );
   }
}


// Entpacke die Kartendatenbank.
void unzip(){
   FILE * fp;
   char * contents;
   //Open the ZIP archive
   int err = 0;
   struct zip * z = zip_open( "../../data/AllSets.json.zip", 0, &err );

   //Search for the file  of given name
   const char * name = "AllSets.json";
   const char * unzipped = "../../data/AllSets.json";
   
   struct zip_stat st;
   zip_stat_init( &st );
   zip_stat( z, name, 0, &st );

   //Alloc memory for its uncompressed contents
   contents = malloc( FILESIZE );

   //Read the compressed file
   struct zip_file * f = zip_fopen( z, name, 0 );
   zip_fread( f, contents, FILESIZE );
   fp = fopen( unzipped, "wb" );
   fwrite( contents, 1, FILESIZE, fp );
   fclose( fp );
   zip_fclose(f);

   //And close the archive
   zip_close( z );

   //delete allocated memory
   free( contents );
}

int main( void )
{
   download();
   unzip();
   return 0;
}
