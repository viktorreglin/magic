/*

Update card prices.

-Read card database
-Cycle through rows, ie cards
	-Use card name & edition to get the latest price --> might need translation from edition code to edition
	-Update the price (and the pricedate)


Next steps:
-Use qdb_query() to get full list of cards back. Each card is then a struct with the name and edition attributes.
-Use this to get the price.
-Think about also storing the pricedate.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> /* Für stat() */
#include <unistd.h> /* für sleep() */
#include <time.h>

/* Einfügen der QDB-Funktionen */
#include "../include/qdb.h"

/* Einfügen der Preisabfrage */
#include "../get_price/get_price.h"



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


/* Updated die Preise aller Karten */
void update_price( cJSON * json, char * db, char * table, time_t start ){
	// ../../bin/qdb_update -p magic2 Card "id = 'c033ad60b21f98f9142611ccc023c1c789f8c617'" pricecent $PRICE pricedate 0 text "Preis geändert"


	qdb_query( char * dbname, char * table, int * nrows, char * filter, bool printquery );


    /* Durch Datenbankzeilen gehen */
    tr = qdb_begin_row( db, table );

    /* Preis abfragen */
    url_string[0] = '\0';
    price_string[0] = '\0';
    price = get_price( name, edition_code_id, url_string, price_string );
    if ( price > 0 ) {
        sprintf( str, "%.2f", price );
        qdb_update( tr, "priceeuro", str );
    }
	qdb_end_row( tr, false );

}


int main(){

    char          * db              = "magic2"; // später in all_cards ändern
    char          * cards_table     = "Card";


    update_price( json, db, cards_table, start );

    exit( 0 );
}
