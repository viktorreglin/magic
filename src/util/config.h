#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef void * CONFIG;

typedef enum { is_unknown, is_int, is_string } VTYPE;


CONFIG config_read( char * filename, char comment );
/*------------------------------------------------------CONFIG cfg------------
  liest Konfigurationsfile
  zeilenweise:
  Name=Value
  Name = String, Value = Int oder String
  name case INsensitiv
  Ab comment wird bis Zeilenende abgeschnitten
  Leerzeichen am Stringanfang und -ende werden abgeschnitten
  - oder String in " " (dann kein " im String)
  return: Zeiger auf interne Config-Struktur (alloziert) oder 0
  Fehler werden nach stderr ausgegebn
  ------------------------------------------------------------------*/

int    config_num( CONFIG cfg ); // Anzahl der Eintrage
VTYPE  config_type  ( CONFIG cfg, char * name );
char * config_string( CONFIG cfg, char * name, char * def ); // def wenn name nicht in cfg
int    config_int   ( CONFIG cfg, char * name, int    def ); // def wenn name nicht in cfg

#endif  // _CONFIG_H_
