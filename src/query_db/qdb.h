#ifndef _QDB_H_tabdef
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
   char * value;
} QDB_TABLEENTRY;


#ifdef TYPEMAPPING_DEFINITION
static QDB_TYPEMAPPING typemapping[] =
{
   { "int"        , "int"                              },
   { "string(%1)" , "varchar(%1) character set utf8"   },
   { "string"     , "varchar(1024) character set utf8" },
   { "ident"      , "binary(40)"                       },
   { "color"      , "set('W','R','G','U','B')"         },
   { "mana"       , "varchar(24)"                      },
   { "date"       , "datetime"                         },
};
// parameter = %X, X = 1 char

static char * stringtypes[] =
{
   "varchar", "set", "binary"
};
#endif


// helper functions
char * sql_type( char * magic_type );
void mysql_print_error( MYSQL * conn, char * txt1, char * txt2 );
QDB_TABLEENTRY * qdb_get_properties( char * dbname, char * tablename, int * pnum );
char * sql_buildquery( char * pattern, char * table, QDB_TABLEENTRY * prop, int num, bool withvalues );


// API  (error Ausgabe über stderr)
typedef MYSQL * QDB_HANDLE;

QDB_HANDLE qdb_open( char * dbname ); // Datenbank öffnen, returns 0 iff error
void       qdb_close( QDB_HANDLE * pqh );


#endif // _QDB_H_
