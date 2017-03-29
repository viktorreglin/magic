// Downloads and unpacks the cards database.

//TEST
#include <stdio.h>
#include <unistd.h> // For chdir().

#define STRLEN 1000


int main(){
   char dir[STRLEN] = "/home/viktor/Documents/MEGAsync/Anderes/Computer/Programming/viktor/C/Magic/git/magic/data/";
   char remove_command[STRLEN] = "rm AllSets.json AllSets.json.zip";
   char wget_command[STRLEN] = "wget https://mtgjson.com/json/AllSets.json.zip";
   char unzip_command[STRLEN] = "unzip AllSets.json.zip";

   chdir( dir );
   system( remove_command );
   system( wget_command );
   system( unzip_command );

   return 0;
}
