// JSON files in lesbare form bringen
// stdin > formjson > stdout

// build: gcc -o formjson formjson.c

#include <stdio.h>
#include "bool.h"


#define indent  2


int main( void )
{
int prevch, ch, nextch, level;
bool instring, escaped, brackets;

   level    = 0;
   instring = false;
   escaped  = false;
   brackets = false;
   prevch   = 0;
   ch = getchar();
   while( ch != EOF )
   {
      nextch = getchar();
      if( escaped )
      {
         putchar( ch );
         escaped = false;
      }
      else if( instring )
      {
         putchar( ch );
         if( ch == '"')
            instring = false;
         else if( ch == '\\' )
            escaped = true;
      }
      else
      {
         switch( ch )
         {
         case '{':
            if( prevch != ',' )
               printf( "\n%*s", level*indent, "" );
            printf( "%c\n%*s", ch, (level+1)*indent, "" );
            brackets = false;
            level++;
            break;
         case '}':
            if( level > 0 )
               level--;
            printf( "\n%*s%c", level*indent, "", ch );
            if( nextch != ',' )
               printf( "\n%*s", level*indent, "" );
            break;
         case ',':
            printf( "%c ", ch );
            if( !brackets )
               printf( "\n%*s", level*indent, "" );
            break;
         case ':':
            printf( "%c ", ch );
            break;
         case '"':
            instring = true;
            putchar( ch );
            break;
         case '[':
            brackets = true;
            putchar( ch );
            break;
         case ']':
            brackets = false;
            putchar( ch );
            break;
         case ' ':
         case '\t':
         case '\n':
         case '\r':
            break;
         default:
            putchar( ch );
         }
      }
      if( (ch != ' ') && (ch != '\n') && (ch != '\t') && (ch != '\r') )
         prevch = ch;
      ch = nextch;
   }
   putchar( '\n' );
   return 0;
}
