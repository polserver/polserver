/** @file
 *
 * @par History
 */


#include <cstring>
#include <iomanip>

#include "../clib/Program/ProgramConfig.h"
#include "../clib/fileutil.h"
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
  std::string ecompile_path =
      Clib::normalized_dir_form( Clib::FullPath( Clib::ProgramConfig::programDir().c_str() ) );
  for ( PC = 0; PC < nLines; PC++ )
  {
    if ( _readToken( token, PC ) )
    {
      return;
    }
    else
    {
      if ( fileline.size() > PC && !fileline[PC].empty() )
      {
        // write relative path to common base between ecompile and script (needed for testscripts
        // and also in general to verbose otherwise)
        std::string file = Clib::normalized_dir_form( fileline[PC], true );
        std::string common = Clib::common_base_dir( file, ecompile_path );
        if ( !common.empty() )
          file.erase( 0, common.size() );
        os << file << std::endl;
      }
      if ( program_PC == PC && !program_decl.empty() )
        os << program_decl << std::endl;
      if ( function_decls.size() > PC && !function_decls[PC].empty() )
        os << function_decls[PC] << std::endl;
      if ( sourcelines.size() > PC && !sourcelines[PC].empty() )
        os << sourcelines[PC] << std::endl;
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
