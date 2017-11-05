#ifndef _QDB_H_
#define _QDB_H_

typedef struct
{
   char * magtype;
   char * sqltype;
} TYPEMAPPING;

#ifdef TYPEMAPPING_DEFINITION
static TYPEMAPPING typemapping[] =
{
   { "int"        , "int"                                           },
   { "string(%1)" , "varchar(%1) character set utf8"                },
   { "string"     , "varchar(1024) character set utf8"              },
   { "ident"      , "binary(20)"                                    },
   { "color"      , "set('White', 'Red', 'Green', 'Blue', 'Black')" },
   { "mana"       , "varchar(24)"                                   },
   { "date"       , "datetime"                                      },
};
// parameter = %X, X = 1 char
#endif

char * sql_type( char * magic_type );


#endif // _QDB_H_
