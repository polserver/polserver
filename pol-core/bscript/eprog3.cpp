/** @file
 *
 * @par History
 */


#include <cstring>
#include <iomanip>

#include "eprog.h"
#include "symcont.h"
#include "token.h"
#include "tokens.h"

namespace Pol
{
namespace Bscript
{
void EScriptProgram::dump( std::ostream& os )
{
  Token token;
  unsigned PC;
  if ( !exported_functions.empty() )
  {
    os << "Exported Functions:" << std::endl;
    os << "   PC  Args  Name" << std::endl;
    for ( auto& elem : exported_functions )
    {
      os << std::setw( 5 ) << elem.PC << std::setw( 6 ) << elem.nargs << "  " << elem.name
         << std::endl;
    }
  }
  unsigned nLines = tokens.length() / sizeof( StoredToken );
  for ( PC = 0; PC < nLines; PC++ )
  {
    if ( _readToken( token, PC ) )
    {
      return;
    }
    else
    {
      os << PC << ": " << token << std::endl;
      if ( token.id == INS_CASEJMP )
      {
        dump_casejmp( os, token );
      }
    }
  }
}

void EScriptProgram::dump_casejmp( std::ostream& os, const Token& token )
{
  const unsigned char* dataptr = token.dataptr;
  for ( ;; )
  {
    unsigned short offset;
    std::memcpy( &offset, dataptr, sizeof( unsigned short ) );
    dataptr += 2;
    unsigned char type = *dataptr;
    dataptr += 1;
    if ( type == CASE_TYPE_LONG )
    {
      int lval;
      std::memcpy( &lval, dataptr, sizeof( int ) );
      dataptr += 4;
      os << "\t" << lval << ": @" << offset << std::endl;
    }
    else if ( type == CASE_TYPE_DEFAULT )
    {
      os << "\tdefault: @" << offset << std::endl;
      break;
    }
    else
    {  // type is the length of the string, otherwise
      os << "\t\"" << std::string( (const char*)dataptr, type ) << "\": @" << offset << std::endl;
      dataptr += type;
    }
  }
}
}  // namespace Bscript
}  // namespace Pol
