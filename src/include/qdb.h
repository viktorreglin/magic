#ifndef _QDB_H_
#define _QDB_H_

#include <stdio.h>
#include <mysql.h>
#include "bool.h"

typedef struct
{
   char * magtype;
   char * sqltype;
} QDB_TYPEMAPPING;

typedef struct
{
   char * name;
   char * sqltype;
   bool   isstring;
   bool   isdate;
   int    key; // 1 = primary, 2 = foreign
   char * value;
} QDB_TABLEENTRY;

typedef struct
{
   char * dbname;
   char * tablename;
   int    numofprop;
   QDB_TABLEENTRY * properties; // array
} QDB_ROWDATA;


typedef struct
{
  char * strval; // Name oder Wert
  int    intval; // wird je nach Typ gefüllt
} QDB_VALUE;
/* in row[0]:
   strval = property name
   intval = type flags (bit encoded):
*/
#define QDB_TYPE_IS_STRING 1
#define QDB_TYPE_IS_DATE   2

typedef struct
{
   int numofvalues;
   QDB_VALUE * value; // value[numofvalues]
} QDB_RESULTROW;   // eine Zeile (oder Überschrift)

typedef struct
{
   int numofrows;
   QDB_RESULTROW * row; // row[numofrows+1], row[0] contains the names
} QDB_RESULT;


#ifdef TYPEMAPPING_DEFINITION
static QDB_TYPEMAPPING typemapping[] =
{
   { "bool"       , "bool"                             },
   { "int"        , "int"                              },
   { "string(%1)" , "varchar(%1) character set utf8"   },
   { "string"     , "varchar(1024) character set utf8" },
   { "ident"      , "binary(40)"                       },
   { "color"      , "set('W','R','G','U','B')"         },
   { "date"       , "datetime"                         },
};
// parameter = %X, X = 1 char

static char * stringtypes[] =
{
   "varchar", "set", "binary"
};
#endif


// helper functions
MYSQL * sql_open( char * dbname ); // Datenbank öffnen, returns 0 iff error
void    sql_close( MYSQL * dh );
char *  sql_type( char * magic_type );
void    sql_print_error( MYSQL * conn, char * txt1, char * txt2 );
char *  sql_buildquery( char * pattern, char * p1, char * p2, QDB_TABLEENTRY * prop, int num, bool withvalues, char * tablename );
// pattern: %s = p1, p2; ! = properties, ? = values

QDB_TABLEENTRY * qdb_get_properties( char * dbname, char * tablename, int * pnum );

// API  (error Ausgabe über stderr)
typedef QDB_ROWDATA * QDB_ROW;

// Erzeugung einer Datenbank
QDB_ROW    qdb_begin_row( char * dbname, char * table ); // Tabellenzeile anfangen
bool       qdb_end_row( QDB_ROW tr ); // Tabellenzeile beenden und eintragen, true iff erfolgreich
bool       qdb_set_value( QDB_ROW tr, char * property, char * value ); // einen Wert eintragen


// Abfrage und Änderung einer Datenbank
QDB_RESULT * qdb_query( char * dbname, char * table, int * nrows, char * filter, bool printquery );
// liefert Ergebnis in allozierter Struktur, Anzahl der Zeilen wird auch über *nrows zurückgegeben, filter: Sprache siehe query_db_if.txt
void qdb_free( QDB_RESULT * presult ); // gibt den Result-Speicher wieder frei


#endif // _QDB_H_
