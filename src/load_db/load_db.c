// Downloads the cards database.

#include <stdio.h>

#define BUFFLEN 1000


int main(){
   char system_command[BUFFLEN] = "wget https://mtgjson.com/json/AllSets.json.zip";

   system( system_command );
   return 0;
}
