// JSON-Datenbank in Editionen und Karten aufteilen

/*

ADD FLAG FOR PRICE DOWNLOAD

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> /* Für stat() */
#include <unistd.h> /* für sleep() */
#include <time.h>

/* Einfügen der JSON library */
#include "../lib/cJSON.h"

/* Einfügen der QDB-Funktionen */
#include "../include/qdb.h"

/* Einfügen der Preisabfrage */
#include "../get_price/get_price.h"


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
    if( cJSON_IsArray ( trait_obj ) ) return 'a';

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


/* Ersetzt Leerzeichen in einem String druch - */
char * space_to_dash( char * str ){
    /* Leerzeichen ersetzen
    for ( char * p = str; ( p = strchr( p, ' ' ) ); ++p ){ // Doppelte Klammern sonst gibt gcc eine Warnung vor Zuweisung
        *p = '-';
    }
    */
    for ( int i = 0; i < strlen( str ); i++ ){
        switch ( str[i] ){
            case ':':
                strcpy( &str[i], &str[i + 1] );

            case ' ':
                str[i] = '-';
        }
    }

    return str;
}


/* Schreibt alle Editionen eines cJSON-Objekts in eine Datei */
void export_editions( cJSON * json, char * db, char * table ){

    int     i;
    cJSON * edt;
    cJSON * obj;
    cJSON * sub_obj;
    char    type_char;
    char    str[12];
    char    array_string[200];

    QDB_ROW tr;

    /* Zu exportierende Eigenschaften von Editionen */
    char  * edt_trait_ids[] = { "name", "code", "gathererCode", "magicCardsInfoCode", "releaseDate", "border", "type", "mkm_name", "mkm_id" };

    /* Erste Edition ist das Kind des gesamten cJSON-Objekts */
    edt = json->child;

    do {
        /* Namen der Edition ausgeben */
        //printf( "Editionen: %s\n", cJSON_GetObjectItemCaseSensitive( edt, "mkm_name" )->valuestring );

        /* Neue Zeile in der Tabelle erzeugen */
        tr = qdb_begin_row( db, table );

        /* Infos der Edition auslesen */
        for( i = 0; i < ( sizeof( edt_trait_ids ) / sizeof( edt_trait_ids[0] ) ); i++ ){

            /* obj ist zB der Editionsname (als cJSON Objekt) */
            obj = trait_obj( edt, edt_trait_ids[i] );

            /* Ist der Wert ein String, eine Zahl oder ein Array? */
            type_char = obj_type( obj );
            switch( type_char ){
                case 's':
                    /* Falls String, dann den String des cJSON-Objekts auslesen */
                    qdb_set_value( tr, edt_trait_ids[i], obj_string( obj ) );
                    break;

                case 'n':
                    /* Falls Zahl, dann den Value des cJSON-Objekts auslesen */
                    sprintf( str, "%d", obj_int( obj ) );
                    qdb_set_value( tr, edt_trait_ids[i], str );
                    break;

                case 'a':
                    /* MOMENTAN NICHT IN BENUTZUNG
                    Falls Array, dann den Array zu einem String zusammenfassen */
                    array_string[0] = '\0';
                    cJSON_ArrayForEach( sub_obj, obj ) {
                        strcat( array_string, obj_string( sub_obj ) );
                    }
                    qdb_set_value( tr, edt_trait_ids[i], array_string );
                    break;
            }

        }

        /* Zeile beenden */
        qdb_end_row( tr, false );

        edt = edt->next;

    } while( edt != NULL );

}


/* Schreibt alle Karten eines cJSON-Objekts in eine Datei */
void export_cards( cJSON * json, char * db, char * table, time_t start ){

    int     i;
    int     j = 0;
    cJSON * edt;
    cJSON * cards;
    cJSON * card;
    cJSON * obj;
    cJSON * sub_obj;
    char    type_char;
    char    str[12];
    char    array_string[200];
    cJSON * edition_json;
    char  * edition;
    QDB_ROW tr;
    float   price;

    // Für die Preisabfrage
    char    price_string[1000];
    char    url_string[1000];

    /* Zu exportierende Eigenschaften von Editionen */
    char  * cards_id    = "cards";
    char  * edt_code_id = "code";   /* Editions-Code wird separat gespeichert um die Karten zuzuordnen */
    char  * edt_name_id = "mkm_name";

    /* Zu exportierende Eigenschaften von Karten */
    char  * card_trait_ids[] = { "artist","cmc","colorIdentity","colors","flavor","id","imageName","layout",
                                "manaCost","mciNumber","multiverseid","name","power","rarity","reserved",
                                "subtypes","supertypes","text","toughness","type","types","edition_code",
                                "datadate","pricedate" };
    char  * edition_code_id = "edition_code";


    /* Erste Edition ist das Kind des gesamten cJSON-Objekts */
    edt = json->child;

    do {
        /* Edition auslesen um den Kartenpreis abzufragen */
        edition_json = cJSON_GetObjectItemCaseSensitive( edt, edt_name_id );
        if ( cJSON_IsString( edition_json ) && edition_json->valuestring != NULL ){
            edition = edition_json->valuestring;
            edition = space_to_dash( edition );
            printf( "\nKarten: %s\n", edition );
        }

        /* Infos der Karten auslesen */
        cards = cJSON_GetObjectItemCaseSensitive( edt, cards_id );

        cJSON_ArrayForEach( card, cards ){

            /* Neue Zeile in der Tabelle erzeugen */
            tr = qdb_begin_row( db, table );

            /* Editions-Code auslesen, wird zur Kartenzuordnung benötigt */
            obj = trait_obj( edt, edt_code_id );
            j += 1;
            qdb_set_value( tr, edition_code_id , obj_string( obj ) );

            for( i = 0; i < ( sizeof( card_trait_ids ) / sizeof( card_trait_ids[0] ) ); i++ ){
                //printf( "%s\n", card_trait_ids[i] );

                /* obj ist zB der Kartenname (als cJSON Objekt) */
                obj = trait_obj( card, card_trait_ids[i] );

                type_char = obj_type( obj );

                switch( type_char ){
                    case 's':
                        /* Falls anderer String, dann den String des cJSON-Objekts auslesen */
                        qdb_set_value( tr, card_trait_ids[i], obj_string( obj ) );

                        /* Falls es der Kartenname ist, dann auch den Preis abfragen */
                        if ( strcmp( card_trait_ids[i], "name" ) == 0 ){
                            printf( "card: %s\n", obj_string( obj ) );
                            ///*
                            url_string[0] = '\0';
                            price_string[0] = '\0';
                            price = get_price( space_to_dash( obj_string( obj ) ), edition, url_string, price_string );
                            if ( price > 0 ) {
                                sprintf( str, "%.2f", price );
                                qdb_set_value( tr, "priceeuro", str );
                            }
                            //*/
                            //sleep( 1 );
                        }
                        break;

                    case 'n':
                        /* Falls Zahl, dann den Value des cJSON-Objekts auslesen */
                        sprintf( str, "%d", obj_int( obj ) );
                        qdb_set_value( tr, card_trait_ids[i], str );
                        break;

                    case 'a':
                        /* Falls Array, dann den Array zu einem String zusammenfassen */
                        array_string[0] = '\0';
                        cJSON_ArrayForEach( sub_obj, obj ) {
                            strcat( array_string, obj_string( sub_obj ) );
                        }
                        qdb_set_value( tr, card_trait_ids[i], array_string );
                        break;
                }

            }

            /* Zeile beenden */
            qdb_end_row( tr, false );
        }
        printf( "Elapsedd time: %.4ld\n", ( time( NULL ) - start ) );
        edt = edt->next;

    } while( edt != NULL );

}



int main(){

    char          * source_filename = "../../data/AllSets.json";
    char          * db              = "all_cards";
    char          * editions_table  = "Edition";
    char          * cards_table     = "Card";

    FILE          * source_file;
    unsigned long   size;
    char          * file_contents;
    cJSON         * json;


    time_t start = time( NULL );

    /* JSON-Datei öffnen */
    source_file = fopen( source_filename, "r" );

    /* Dateigröße bestimmen */
    size = filesize( source_filename );

    /* Datei als String einlesen */
    file_contents = malloc( size + 1 );
    read_source( source_file, size, file_contents );

    /* JSON parsen */
    json = cJSON_Parse( file_contents );

    /* Alte Datenbank löschen */
    system( "bash -c '../../bin/qdb_delete all_cards'" );

    /* Neue Datenbank anlegen */
    system( "../../bin/qdb_create all_cards all_cards.tabledef" );

    /* Editionen in Datenbank schreiben */
    printf( "\nStarting editions export\n" );
    printf( "Elapsed time: %.4ld\n", ( time( NULL ) - start ) );
    export_editions( json, db, editions_table );

    /* Karten in Datenbank schreiben */
    printf( "\nStarting cards export\n" );
    printf( "Elapsed time: %.4ld\n", ( time( NULL ) - start ) );
    export_cards( json, db, cards_table, start );

    /* Aufräumen */
    cJSON_Delete( json );
    free( file_contents );

    exit( 0 );
}
