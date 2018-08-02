// JSON-Datenbank in Editionen und Karten aufteilen

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> /* Für stat() */

/* Einfügen der JSON library */
#include "../lib/cJSON.h"
#include "../lib/cJSON.c"


/* Gibt ein Unterobjekt einer Karte/Edition an, zB den Kartennamen (als cJSON Objekt). */
cJSON * trait_obj( cJSON * obj, char * trait_id ){
    return cJSON_GetObjectItemCaseSensitive( obj, trait_id );
}


/* 
    Gibt an, ob ein Objekt ein String oder eine Zahl ist.
    cJSON->type speichert nur eine bit-flag, ist daher unpraktisch für Vergleiche.
*/
char obj_type( cJSON * trait_obj){

    if( cJSON_IsString( trait_obj ) ) return 's';
    if( cJSON_IsNumber( trait_obj ) ) return 'n';

    return 0;
}


/* Gibt cJSON valuestring an. */
char * obj_string( cJSON * trait_obj ){
    return trait_obj->valuestring;
}


/* Gibt cJSON valueint an. */
int obj_int( cJSON * trait_obj ){
    return trait_obj->valueint;
}


/* Bestimmt die Größe einer Datei */
//include in gcc call:
// -I../include -L../lib -lutil
unsigned long filesize( char * filename ){
    struct stat st;

    stat( filename, &st );
    return st.st_size;
}


/* Liest eine Datei als String ein */
void read_source( FILE * source_file, unsigned long size, char * string_dest ){
    fread( string_dest, size , 1, source_file );
    string_dest[size + 1] = '\0';
}


/* Schreibt alle Editionen eines cJSON-Objekts in eine Datei */
void export_editions( cJSON * json, FILE * dest_file ){
    
    int     i;
    cJSON * edt;
    cJSON * obj;

    /* Zu exportierende Eigenschaften von Editionen */
    char  * edt_trait_ids[]     = { "border", "name", "code" };


    /* Erste Edition ist das Kind des gesamten cJSON-Objekts */
    edt = json->child;

    do {

        /* Infos der Edition auslesen */
        for( i = 0; i < ( sizeof( edt_trait_ids ) / sizeof( edt_trait_ids[0] ) ); i++ ){
                
            /* obj ist zB der Editionsname (als cJSON Objekt) */
            obj = trait_obj( edt, edt_trait_ids[i] );

            fprintf( dest_file, "%s \"%s\" ", edt_trait_ids[i], obj_string( obj ) );

        }

        fprintf( dest_file, "\n" );

        edt = edt->next;
    } while( edt != NULL );

}


/* Schreibt alle Karten eines cJSON-Objekts in eine Datei */
void export_cards( cJSON * json, FILE * dest_file ){
    
    int     i, j;
    cJSON * edt;
    cJSON * cards;
    cJSON * card;
    cJSON * obj;
    cJSON * sub_obj;
    char    type_char;

    /* Zu exportierende Eigenschaften von Editionen */
    char  * cards_id    = "cards";
    char  * edt_code_id = "code";   /* Editions-Code wird separat gespeichert um die Karten zuzuordnen */
    char  * edt_id      = "Edition";
    char    edt_code_string[3];     /* Editions-Codes sind immer 3 Zeichen lang */

    
    /* Zu exportierende Eigenschaften von Karten */
    char  * card_trait_ids[] = { "cmc", "colorIdentity", "colors", "id", "manaCost", "name", "power", "rarity", "subtypes", "text", "toughness", "type", "types" };


    /* Erste Edition ist das Kind des gesamten cJSON-Objekts */
    edt = json->child;

    do {

        /* Editions-Code auslesen, wird zur Kartenzuordnung benötigt */
        obj = trait_obj( edt, edt_code_id );
        sprintf( edt_code_string, "%s", obj_string( obj ) );

        /* Infos der Karten auslesen */
        cards    = cJSON_GetObjectItemCaseSensitive( edt, cards_id );
        cJSON_ArrayForEach( card, cards ){

            fprintf( dest_file, "%s \"%s\" ", edt_id, edt_code_string );

            for( i = 0; i < ( sizeof( card_trait_ids ) / sizeof( card_trait_ids[0] ) ); i++ ){
                
                /* obj ist zB der Kartenname (als cJSON Objekt) */
                obj         = trait_obj( card, card_trait_ids[i] );

                /* Eigenschaften wie 'colors' bestehen aus Arrays */
                if( cJSON_IsArray( obj ) ){
                    fprintf( dest_file, "%s", card_trait_ids[i] );
                    cJSON_ArrayForEach( sub_obj, obj ) fprintf( dest_file, " \"%s\" ", obj_string( sub_obj ) );
                }

                type_char   = obj_type( obj );

                if     ( type_char == 's' ) fprintf( dest_file, "%s \"%s\" ", card_trait_ids[i], obj_string( obj ) ); /* Im 'text' sind teilweise Zeilenumbrüche drin */
                else if( type_char == 'n' ) fprintf( dest_file, "%s \"%d\" ", card_trait_ids[i], obj_int( obj ) );
            
            }

            fprintf( dest_file, "\n" );
        }

        edt = edt->next;
    } while( edt != NULL );

}



int main( int argc, char *argv[] ){

    char          * source_filename   = "../../data/AllSets.json";
    char          * editions_filename = "Editions.txt";
    char          * cards_filename    = "Cards.txt";

    FILE          * source_file;
    FILE          * editions_file;
    FILE          * cards_file;

    unsigned long   size;
    char          * file_contents;
    cJSON         * json;


    /* JSON-Datei öffnen */
    source_file = fopen( source_filename, "r" );

    /* Dateigröße bestimmen */
    size = filesize( source_filename );

    /* Datei als String einlesen */
    file_contents = malloc( size + 1 );
    read_source( source_file, size, file_contents );

    /* JSON parsen */
    json = cJSON_Parse( file_contents );


    /* Editionen in Datei schreiben */
    editions_file = fopen( editions_filename, "w" );
    export_editions( json, editions_file );
    fclose( editions_file );


    /* Karten in Datei schreiben */
    cards_file = fopen( cards_filename, "w" );
    export_cards( json, cards_file );
    fclose( cards_file );


    /* Aufräumen */
    cJSON_Delete( json );
    free( file_contents );

    exit( 0 );
}
