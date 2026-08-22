/** @file
 *
 * @par History
 */

#include "bscript/eprog.h"

#include <cstdio>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iomanip>

#include "clib/refptr.h"
#include "clib/stlutil.h"
#include "bscript/escriptv.h"
#include "bscript/fmodule.h"


namespace Pol::Bscript
{
EScriptProgram::EScriptProgram()
    : ref_counted(),
      nglobals( 0 ),
      expectedArgs( 0 ),
      haveProgram( false ),
      name( "" ),
      modules(),
      tokens(),
      symbols(),
      exported_functions(),
      version( 0 ),
      invocations( 0 ),
      instr_cycles( 0 ),
      pkg( nullptr ),
      instr(),
      debug_loaded( false ),
      globalvarnames(),
      blocks(),
      dbg_functions(),
      dbg_filenames(),
      dbg_filenum(),
      dbg_linenum(),
      dbg_ins_blocks(),
      dbg_ins_statementbegin()
{
  ++escript_program_count;

  // compiler only:
  EPDbgBlock block;
  block.parentblockidx = 0;
  block.parentvariables = 0;
  blocks.push_back( block );
}

EScriptProgram::~EScriptProgram()
{
  Clib::delete_all( modules );
  --escript_program_count;
}

std::string EScriptProgram::dbg_get_instruction( size_t atPC ) const
{
  return fmt::to_string( instr[atPC].token );
}

size_t EScriptProgram::sizeEstimate() const
{
  using namespace Clib;
  size_t size = sizeof( EScriptProgram );
  // memsize() of a container of strings already counts each string's capacity.
  size += memsize( globalvarnames ) + memsize( dbg_filenames ) + memsize( dbg_filenum ) +
          memsize( dbg_linenum ) + memsize( dbg_ins_blocks ) + memsize( dbg_ins_statementbegin ) +
          memsize( modules ) + memsize( instr );

  // The bulk of a compiled script, and memsize() cannot see it: both hold a raw buffer.
  size += tokens.allocated() + symbols.allocated();

  // A string or a vector nested inside an element is invisible to memsize() as well, so these
  // three say what one element costs.
  size += memsize( exported_functions,
                   []( const auto& f ) { return sizeof( f ) + f.name.capacity(); } );
  size += memsize( dbg_functions, []( const auto& f ) { return sizeof( f ) + f.name.capacity(); } );
  size +=
      memsize( blocks, []( const auto& b ) { return sizeof( b ) + memsize( b.localvarnames ); } );

  return size;
}

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

    fmt::println( os, "{}: {}", PC, token );
    if ( token.id == INS_CASEJMP )
    {
      dump_casejmp( os, token );
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
}  // namespace Pol::Bscript
