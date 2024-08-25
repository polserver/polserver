/** @file
 *
 * @par History
 */


#include <stdio.h>
#include <string.h>
#include <string>

#include "../clib/clib.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "../clib/stlutil.h"
#include "compctx.h"
// EPROG compiler-only functions
#include "eprog.h"
#include "filefmt.h"
#include "fmodule.h"
#include "symcont.h"
#include "token.h"
#include "tokens.h"
#include "userfunc.h"

namespace Pol
{
namespace Bscript
{
extern int include_debug;

void EScriptProgram::erase()
{
  tokens.erase();
  symbols.erase();
  nglobals = 0;
  dbg_filenum.clear();
  dbg_linenum.clear();
  globalvarnames.clear();
  blocks.clear();
  dbg_functions.clear();
  dbg_ins_blocks.clear();
  dbg_ins_statementbegin.clear();
}

void EScriptProgram::update_dbg_pos( const Token& token )
{
  if ( token.dbg_filenum != 0 )
  {
    curfile = token.dbg_filenum;
    curline = token.dbg_linenum;
  }
}

void EScriptProgram::add_ins_dbg_info()
{
  dbg_filenum.push_back( curfile );
  dbg_linenum.push_back( curline );
  dbg_ins_blocks.push_back( curblock );
  dbg_ins_statementbegin.push_back( statementbegin );
  statementbegin = false;
  passert( tokens.count() == dbg_filenum.size() );
}

void EScriptProgram::setcontext( const CompilerContext& ctx )
{
  curfile = ctx.dbg_filenum;
  curline = ctx.line;
}

void EScriptProgram::append( const StoredToken& stoken )
{
  tokens.append_tok( stoken, nullptr );

  add_ins_dbg_info();
}

void EScriptProgram::append( const StoredToken& stoken, unsigned* posn )
{
  tokens.append_tok( stoken, posn );

  add_ins_dbg_info();
}

void EScriptProgram::append( const StoredToken& stoken, const CompilerContext& ctx )
{
  tokens.append_tok( stoken, nullptr );

  setcontext( ctx );

  add_ins_dbg_info();
}

void EScriptProgram::append( const StoredToken& stoken, const CompilerContext& ctx, unsigned* posn )
{
  tokens.append_tok( stoken, posn );

  setcontext( ctx );

  add_ins_dbg_info();
}

void EScriptProgram::addToken( const Token& token )
{
  unsigned tokpos = 0;
  if ( token.id > 255 )
  {
    throw std::runtime_error( "Trying to write an illegal token" );
  }

  update_dbg_pos( token );

  switch ( token.type )
  {
  case TYP_OPERAND:  // is variable name, long, double, string lit
  {
    unsigned sympos = 0;
    BTokenType type = token.type;
    switch ( token.id )
    {
    case TOK_LONG:
      symbols.append( token.lval, sympos );
      break;
    case TOK_DOUBLE:
      symbols.append( token.dval, sympos );
      break;
    case TOK_STRING:  // string literal
      symbols.append( token.tokval(), sympos );
      break;
    case INS_ADDMEMBER2:
    case INS_ADDMEMBER_ASSIGN:
    case TOK_IDENT:  // unclassified (ick)
      symbols.append( token.tokval(), sympos );
      break;
    case TOK_GLOBALVAR:
    case TOK_LOCALVAR:
      sympos = token.lval;
      break;
    case TOK_FUNCREF:
      // num params in type
      type = static_cast<BTokenType>( token.userfunc->parameters.size() );
    default:
      break;
    }
    tokens.append_tok( StoredToken( token.module, token.id, type, sympos ), &tokpos );
    if ( token.id == TOK_FUNCREF )
      token.userfunc->forward_callers.push_back( tokpos );
  }
  break;

  case TYP_METHOD:
  {
    if ( token.id != INS_CALL_METHOD_ID )
    {
      unsigned sympos = 0;
      passert( token.tokval() );

      symbols.append( token.tokval(), sympos );

      tokens.append_tok(
          StoredToken(
              token.module, token.id,
              static_cast<BTokenType>(
                  token.lval ),  // # of params, stored in Token.lval, saved in StoredToken.type
              sympos ),
          &tokpos );
    }
    else
    {
      tokens.append_tok( StoredToken( token.module, token.id, static_cast<BTokenType>( token.lval ),
                                      static_cast<unsigned>( token.dval ) ),
                         &tokpos );
    }
  }
  break;
  case TYP_FUNC:
  {
    unsigned sympos = 0;
    if ( include_debug )
    {
      if ( token.tokval() )
        symbols.append( token.tokval(), sympos );
    }
    tokens.append_tok(
        StoredToken(
            token.module, token.id,
            static_cast<BTokenType>(
                token.lval ),  // function index, stored in Token.lval, saved in StoredToken.type
            sympos ),
        &tokpos );
  }
  break;

  case TYP_RESERVED:
  case TYP_OPERATOR:
  case TYP_UNARY_OPERATOR:
    if ( token.id == TOK_ARRAY_SUBSCRIPT )
    {
      // array subscript operators store their
      // array index number in offset
      tokens.append_tok( StoredToken( token.module, token.id, token.type, token.lval ) );
    }
    else if ( token.id == INS_GET_MEMBER || token.id == INS_SET_MEMBER ||
              token.id == INS_SET_MEMBER_CONSUME )
    {
      unsigned sympos = 0;
      symbols.append( token.tokval(), sympos );
      tokens.append_tok( StoredToken( token.module, token.id, token.type, sympos ), &tokpos );
    }
    else
    {
      tokens.append_tok( StoredToken( token.module, token.id, token.type, token.lval ), &tokpos );
    }
    break;
  case TYP_CONTROL:
    switch ( token.id )
    {
    case CTRL_MAKELOCAL:
      tokens.append_tok( StoredToken( token.module, token.id, token.type ) );
      break;

    case CTRL_JSR_USERFUNC:
      tokens.append_tok( StoredToken( token.module, token.id, token.type, 0 ), &tokpos );
      token.userfunc->forward_callers.push_back( tokpos );
      break;

    case INS_POP_PARAM_BYREF:
    case INS_POP_PARAM:
    {
      unsigned sympos = 0;
      symbols.append( token.tokval(), sympos );
      tokens.append_tok( StoredToken( token.module, token.id, token.type, sympos ) );
    }
    break;

    case INS_CHECK_MRO:
    case INS_SKIPIFTRUE_ELSE_CONSUME:
    {
      unsigned sympos = token.lval;
      tokens.append_tok( StoredToken( token.module, token.id, token.type, sympos ) );
    }
    break;

    default:
      ERROR_PRINTLN( "AddToken: Can't handle TYP_CONTROL: {}", token );
      throw std::runtime_error( "Unexpected token in AddToken() (1)" );
      break;
    }
    break;
  default:
    ERROR_PRINTLN( "AddToken: Can't handle {}", token );
    throw std::runtime_error( "Unexpected Token passed to AddToken() (2)" );
    break;
  }

  add_ins_dbg_info();
}

int EScriptProgram::write( const char* fname )
{
  EScriptProgram& program = *this;
  FILE* fp = fopen( fname, "wb" );
  if ( !fp )
    return -1;

  BSCRIPT_FILE_HDR hdr;
  hdr.magic2[0] = BSCRIPT_FILE_MAGIC0;
  hdr.magic2[1] = BSCRIPT_FILE_MAGIC1;
  hdr.version = ESCRIPT_FILE_VER_CURRENT;  // auto-set to latest version (see filefmt.h)
  hdr.globals = static_cast<unsigned short>( globalvarnames.size() );
  fwrite( &hdr, sizeof hdr, 1, fp );

  BSCRIPT_SECTION_HDR sechdr;

  if ( haveProgram )
  {
    BSCRIPT_PROGDEF_HDR progdef_hdr;
    memset( &progdef_hdr, 0, sizeof progdef_hdr );
    sechdr.type = BSCRIPT_SECTION_PROGDEF;
    sechdr.length = sizeof progdef_hdr;
    fwrite( &sechdr, sizeof sechdr, 1, fp );
    progdef_hdr.expectedArgs = expectedArgs;
    fwrite( &progdef_hdr, sizeof progdef_hdr, 1, fp );
  }

  for ( auto& module : program.modules )
  {
    sechdr.type = BSCRIPT_SECTION_MODULE;
    sechdr.length = 0;
    fwrite( &sechdr, sizeof sechdr, 1, fp );

    BSCRIPT_MODULE_HDR modhdr;
    memset( &modhdr, 0, sizeof modhdr );
    Clib::stracpy( modhdr.modulename, module->modulename.get().c_str(), sizeof modhdr.modulename );
    modhdr.nfuncs = static_cast<unsigned int>( module->used_functions.size() );
    fwrite( &modhdr, sizeof modhdr, 1, fp );
    for ( auto& module_func : module->used_functions )
    {
      BSCRIPT_MODULE_FUNCTION func;
      memset( &func, 0, sizeof func );
      passert( module_func->used );

      Clib::stracpy( func.funcname, module_func->name.get().c_str(), sizeof func.funcname );
      func.nargs = static_cast<unsigned char>( module_func->nargs );

      fwrite( &func, sizeof func, 1, fp );
    }
  }

  sechdr.type = BSCRIPT_SECTION_CODE;
  sechdr.length = program.tokens.get_write_length();
  fwrite( &sechdr, sizeof sechdr, 1, fp );
  program.tokens.write( fp );

  sechdr.type = BSCRIPT_SECTION_SYMBOLS;
  sechdr.length = program.symbols.get_write_length();
  fwrite( &sechdr, sizeof sechdr, 1, fp );
  program.symbols.write( fp );

  if ( !exported_functions.empty() )
  {
    BSCRIPT_EXPORTED_FUNCTION bef;
    sechdr.type = BSCRIPT_SECTION_EXPORTED_FUNCTIONS;
    sechdr.length = static_cast<unsigned int>( exported_functions.size() * sizeof bef );
    fwrite( &sechdr, sizeof sechdr, 1, fp );
    for ( auto& elem : exported_functions )
    {
      Clib::stracpy( bef.funcname, elem.name.c_str(), sizeof bef.funcname );
      bef.nargs = elem.nargs;
      bef.PC = elem.PC;
      fwrite( &bef, sizeof bef, 1, fp );
    }
  }

  fclose( fp );
  return 0;
}


EScriptProgramCheckpoint::EScriptProgramCheckpoint( const EScriptProgram& prog )
{
  commit( prog );
}

void EScriptProgramCheckpoint::commit( const EScriptProgram& prog )
{
  module_count = static_cast<unsigned int>( prog.modules.size() );
  tokens_count = prog.tokens.count();
  symbols_length = prog.symbols.length();
  sourcelines_count = static_cast<unsigned int>( prog.sourcelines.size() );
  fileline_count = static_cast<unsigned int>( prog.fileline.size() );
}

unsigned EScriptProgram::varcount( unsigned block )
{
  unsigned cnt = static_cast<unsigned int>( blocks[block].localvarnames.size() );
  if ( block != 0 )
  {
    cnt += varcount( blocks[block].parentblockidx );
  }
  return cnt;
}
unsigned EScriptProgram::parentvariables( unsigned parent )
{
  unsigned cnt = 0;
  if ( parent != 0 )
  {
    cnt = blocks[parent].parentvariables + parentvariables( blocks[parent].parentblockidx );
  }
  return cnt;
}
int EScriptProgram::write_dbg( const char* fname, bool gen_txt )
{
  FILE* fp = fopen( fname, "wb" );
  if ( !fp )
    return -1;

  FILE* fptext = nullptr;
  if ( gen_txt )
    fptext = fopen( ( std::string( fname ) + ".txt" ).c_str(), "wt" );

  u32 count;

  // just a version number
  count = 3;
  fwrite( &count, sizeof count, 1, fp );

  count = static_cast<unsigned int>( dbg_filenames.size() );
  fwrite( &count, sizeof count, 1, fp );
  for ( unsigned i = 0; i < dbg_filenames.size(); ++i )
  {
    if ( fptext )
      fprintf( fptext, "File %u: %s\n", i, dbg_filenames[i].c_str() );
    count = static_cast<unsigned int>( dbg_filenames[i].size() + 1 );
    fwrite( &count, sizeof count, 1, fp );
    fwrite( dbg_filenames[i].c_str(), count, 1, fp );
  }
  count = static_cast<unsigned int>( globalvarnames.size() );
  fwrite( &count, sizeof count, 1, fp );
  for ( unsigned i = 0; i < globalvarnames.size(); ++i )
  {
    if ( fptext )
      fprintf( fptext, "Global %u: %s\n", i, globalvarnames[i].c_str() );
    count = static_cast<unsigned int>( globalvarnames[i].size() + 1 );
    fwrite( &count, sizeof count, 1, fp );
    fwrite( globalvarnames[i].c_str(), count, 1, fp );
  }
  count = tokens.count();
  fwrite( &count, sizeof count, 1, fp );
  for ( unsigned i = 0; i < tokens.count(); ++i )
  {
    if ( fptext )
      fprintf( fptext, "Ins %u: File %u, Line %u, Block %u %s\n", i, dbg_filenum[i], dbg_linenum[i],
               dbg_ins_blocks[i], ( dbg_ins_statementbegin[i] ? "StatementBegin" : "" ) );
    BSCRIPT_DBG_INSTRUCTION ins;
    ins.filenum = dbg_filenum[i];
    ins.linenum = dbg_linenum[i];
    ins.blocknum = dbg_ins_blocks[i];
    ins.statementbegin = dbg_ins_statementbegin[i] ? 1 : 0;
    ins.rfu1 = 0;
    ins.rfu2 = 0;
    fwrite( &ins, sizeof ins, 1, fp );
  }
  count = static_cast<unsigned int>( blocks.size() );
  fwrite( &count, sizeof count, 1, fp );
  for ( unsigned i = 0; i < blocks.size(); ++i )
  {
    const EPDbgBlock& block = blocks[i];
    if ( fptext )
    {
      fprintf( fptext, "Block %u:\n", i );
      fprintf( fptext, "  Parent block: %u\n", block.parentblockidx );
    }

    u32 tmp;
    tmp = block.parentblockidx;
    fwrite( &tmp, sizeof tmp, 1, fp );
    tmp = block.parentvariables;
    fwrite( &tmp, sizeof tmp, 1, fp );

    count = static_cast<unsigned int>( block.localvarnames.size() );
    fwrite( &count, sizeof count, 1, fp );

    unsigned int varfirst = block.parentvariables;
    unsigned int varlast = static_cast<unsigned int>( varfirst + block.localvarnames.size() - 1 );
    if ( varlast >= varfirst )
    {
      if ( fptext )
        fprintf( fptext, "  Local variables %u-%u: \n", varfirst, varlast );
      for ( unsigned j = 0; j < block.localvarnames.size(); ++j )
      {
        std::string varname = block.localvarnames[j];
        if ( fptext )
          fprintf( fptext, "      %u: %s\n", varfirst + j, varname.c_str() );

        count = static_cast<unsigned int>( varname.size() + 1 );
        fwrite( &count, sizeof count, 1, fp );
        fwrite( varname.c_str(), count, 1, fp );
      }
    }
  }
  count = static_cast<unsigned int>( dbg_functions.size() );
  fwrite( &count, sizeof count, 1, fp );
  for ( unsigned i = 0; i < dbg_functions.size(); ++i )
  {
    const EPDbgFunction& func = dbg_functions[i];
    if ( fptext )
    {
      fprintf( fptext, "Function %u: %s\n", i, func.name.c_str() );
      fprintf( fptext, "  FirstPC=%u, lastPC=%u\n", func.firstPC, func.lastPC );
    }
    count = static_cast<unsigned int>( func.name.size() + 1 );
    fwrite( &count, sizeof count, 1, fp );
    fwrite( func.name.c_str(), count, 1, fp );
    u32 tmp;
    tmp = func.firstPC;
    fwrite( &tmp, sizeof tmp, 1, fp );
    tmp = func.lastPC;
    fwrite( &tmp, sizeof tmp, 1, fp );
  }

  if ( fptext )
    fclose( fptext );
  fptext = nullptr;
  fclose( fp );
  fp = nullptr;

  return 0;
}

int EScriptProgram::add_dbg_filename( const std::string& filename )
{
  for ( unsigned i = 0; i < dbg_filenames.size(); ++i )
  {
    if ( dbg_filenames[i] == filename )
      return i;
  }

  // file 0 is nothing
  if ( dbg_filenames.empty() )
    dbg_filenames.push_back( "" );

  dbg_filenames.push_back( filename );
  return static_cast<int>( dbg_filenames.size() - 1 );
}

std::string EScriptProgram::dbg_get_instruction( size_t atPC ) const
{
  OSTRINGSTREAM os;
  os << instr[atPC].token;
  return OSTRINGSTREAM_STR( os );
}


void EScriptProgram::setstatementbegin()
{
  statementbegin = true;
}
void EScriptProgram::enterfunction()
{
  savecurblock = curblock;
  curblock = 0;
}
void EScriptProgram::leavefunction()
{
  curblock = savecurblock;
}
void EScriptProgram::enterblock()
{
  EPDbgBlock block;
  if ( blocks.empty() )
  {
    block.parentblockidx = 0;
    block.parentvariables = 0;
    blocks.push_back( block );
    curblock = 0;
  }

  block.parentblockidx = curblock;
  block.parentvariables = varcount( curblock );

  curblock = static_cast<unsigned int>( blocks.size() );
  blocks.push_back( block );
}
void EScriptProgram::leaveblock()
{
  passert( curblock );
  bool remove = blocks[curblock].localvarnames.empty() && curblock == blocks.size() - 1;
  curblock = blocks[curblock].parentblockidx;
  if ( remove )
  {
    blocks.pop_back();
    for ( auto& elem : dbg_ins_blocks )
    {
      if ( elem >= blocks.size() )
        elem = curblock;
    }
  }
}
void EScriptProgram::addlocalvar( const std::string& localvarname )
{
  blocks[curblock].localvarnames.push_back( localvarname );
}
void EScriptProgram::addfunction( std::string funcname, unsigned firstPC, unsigned lastPC )
{
  EPDbgFunction func;
  func.name = funcname;
  func.firstPC = firstPC;
  func.lastPC = lastPC;
  dbg_functions.push_back( func );
}

size_t EScriptProgram::sizeEstimate() const
{
  using namespace Clib;
  size_t size = sizeof( EScriptProgram ) + program_decl.capacity();
  size += memsize( sourcelines );
  for ( const auto& l : sourcelines )
    size += l.capacity();
  size += memsize( fileline );
  for ( const auto& l : fileline )
    size += l.capacity();
  size += memsize( function_decls );
  for ( const auto& l : function_decls )
    size += l.capacity();
  size += memsize( globalvarnames );
  for ( const auto& l : globalvarnames )
    size += l.capacity();
  size += memsize( dbg_filenames );
  for ( const auto& l : dbg_filenames )
    size += l.capacity();
  size += memsize( dbg_filenum ) + memsize( dbg_linenum ) + memsize( dbg_ins_blocks ) +
          memsize( dbg_ins_statementbegin ) + memsize( modules ) + memsize( exported_functions ) +
          memsize( instr ) + memsize( blocks ) + memsize( dbg_functions );

  return size;
}
}  // namespace Bscript
}  // namespace Pol
