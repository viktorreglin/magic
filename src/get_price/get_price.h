// Preissssss

#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define FILELOCATION "../../data/cardprice.html"

void get_url( char * card, char * edition, char * url );

int download_page( FILE * fp, char * url, long max_seconds );

long filelength( FILE * fp );

int price_string( FILE * fp, char * price );

char * replace_char( char * str, char find, char replace );

float price_to_number( char * price_string );

char * format_name( char * name );

float get_price( char * card, char * edition, char * url, char * price, long max_seconds );
