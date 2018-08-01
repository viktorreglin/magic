// JSON-Datenbank in Editionen und Karten aufteilen

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> /* Für stat() */

/*
    Einfügen der JSON library
    ??? Wie sollte man das effizienter gestalten ???
*/
#include "../../lib/cJSON.h"
#include "../../lib/cJSON.c"

#define FILENAME "SomeSets.json"

int main(){

    FILE          * json_file;
    struct          stat st;
    unsigned long   size;
    char          * file_contents;
    cJSON         * json;
    cJSON         * edt;
    cJSON         * edt_name;
    cJSON         * edt_code;
    cJSON         * edt_border;

    /* Zu exportierende Eigenschaften von Editionen */
    const char    * edt_string      = "../../bin/qdb_insert magic2 Edition"; /* Als Beispiel */
    const char    * edt_name_id     = "name";
    const char    * edt_code_id     = "code";
    const char    * edt_border_id   = "border";

    /* Datei öffnen */
    json_file = fopen( FILENAME, "r" );

    /*
        Dateigröße bestimmen unter Linux 
        ??? Wie geht das mit util ???
    */
    stat( FILENAME, &st );
    size = st.st_size;

    /* Datei als String einlesen */
    file_contents = malloc( size + 1 );
    fread( file_contents, size, 1, json_file );
    file_contents[size + 1] = '\0';

    /* JSON parsen */
    json = cJSON_Parse( file_contents );

    /* Infos aller Edition drucken */
    edt             = json->child;

    do {
        edt_name    = cJSON_GetObjectItemCaseSensitive( edt, edt_name_id );
        edt_code    = cJSON_GetObjectItemCaseSensitive( edt, edt_code_id );
        edt_border  = cJSON_GetObjectItemCaseSensitive( edt, edt_border_id );
        printf("%s %s \"%s\" %s \"%s\" %s \"%s\"\n", edt_string
                                                   , edt_name->string, edt_name->valuestring
                                                   , edt_code->string, edt_code->valuestring
                                                   , edt_border->string, edt_border->valuestring
        );
        edt = edt->next;
    } while( edt != NULL );



    /* Aufräumen */
    cJSON_Delete( json );
    free( file_contents );
    fclose( json_file );

}
