/** @file
 *
 * @par History
 * - 2005/12/07 MuadDib: Added "Recompile required" to Bad version reports.
 */

#include <cstdio>
#include <exception>
#include <string>

#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../clib/strutil.h"
#include "eprog.h"
#include "executor.h"
#include "filefmt.h"
#include "fmodule.h"
#include "modules.h"
#include "objmethods.h"
#include "symcont.h"
#include "token.h"
#include "tokens.h"

namespace Pol
{
namespace Bscript
{
/**
 * Opens and ECL file containing bytecode and reads it
 *
 * This is where script bytecode processing is done
 */
int EScriptProgram::read( const char* fname )
{
  FILE* fp = nullptr;

  try
  {
    name = fname;

    fp = fopen( fname, "rb" );
    if ( !fp )
      throw std::runtime_error( std::string( "Unable to open " ) + fname + " for reading." );

    BSCRIPT_FILE_HDR hdr;
    if ( fread( &hdr, sizeof hdr, 1, fp ) != 1 )
    {
      ERROR_PRINT << "Error loading script " << fname << ": error reading header\n";
      fclose( fp );
      return -1;
    }
    if ( hdr.magic2[0] != BSCRIPT_FILE_MAGIC0 || hdr.magic2[1] != BSCRIPT_FILE_MAGIC1 )
    {
      ERROR_PRINT << "Error loading script " << fname << ": bad magic value '" << hdr.magic2[0]
                  << hdr.magic2[1] << "'\n";
      fclose( fp );
      return -1;
    }
    // auto-check for latest version (see filefmt.h for setting)
    if ( hdr.version != ESCRIPT_FILE_VER_CURRENT )
    {
      ERROR_PRINT << "Error loading script " << fname << ": Recompile required. Bad version number "
                  << hdr.version << "\n";
      fclose( fp );
      return -1;
    }
    version = hdr.version;
    nglobals = hdr.globals;
    BSCRIPT_SECTION_HDR sechdr;
    while ( fread( &sechdr, sizeof sechdr, 1, fp ) == 1 )
    {
      switch ( sechdr.type )
      {
      case BSCRIPT_SECTION_PROGDEF:
        if ( read_progdef_hdr( fp ) )
        {
          ERROR_PRINT << "Error loading script " << fname << ": error reading progdef section\n";
          fclose( fp );
          return -1;
        }
        break;

      case BSCRIPT_SECTION_MODULE:
        if ( read_module( fp ) )
        {
          ERROR_PRINT << "Error loading script " << fname << ": error reading module section\n";
          fclose( fp );
          return -1;
        }
        break;
      case BSCRIPT_SECTION_CODE:
        tokens.read( fp );
        break;
      case BSCRIPT_SECTION_SYMBOLS:
        symbols.read( fp );
        break;
      case BSCRIPT_SECTION_GLOBALVARNAMES:
        if ( read_globalvarnames( fp ) )
        {
          ERROR_PRINT << "Error loading script " << fname
                      << ": error reading global variable name section\n";
          fclose( fp );
          return -1;
        }
        break;
      case BSCRIPT_SECTION_EXPORTED_FUNCTIONS:
        if ( read_exported_functions( fp, &sechdr ) )
        {
          ERROR_PRINT << "Error loading script " << fname
                      << ": error reading exported functions section\n";
          fclose( fp );
          return -1;
        }
        break;
      default:
        ERROR_PRINT << "Error loading script " << fname << ": unknown section type " << sechdr.type
                    << "\n";
        fclose( fp );
        return -1;
      }
    }
    fclose( fp );

    return create_instructions();
  }
  catch ( std::exception& ex )
  {
    ERROR_PRINT << "Exception caught while loading script " << fname << ": " << ex.what() << "\n";
    if ( fp != nullptr )
      fclose( fp );
    return -1;
  }
#ifndef WIN32
  catch ( ... )
  {
    ERROR_PRINT << "Exception caught while loading script " << fname << "\n";
    if ( fp != nullptr )
      fclose( fp );
    return -1;
  }
#endif
}
int EScriptProgram::create_instructions()
{
  int nLines = tokens.length() / sizeof( StoredToken );
  instr.resize( nLines );  // = new Instruction[ nLines ];

  for ( int i = 0; i < nLines; i++ )
  {
    Instruction& ins = instr[i];
    if ( _readToken( ins.token, i ) )
      return -1;

    // executor only:
    ins.func = Executor::GetInstrFunc( ins.token );
  }
  return 0;
}

/**
 * Reads the Program Header section from the file pointer
 */
int EScriptProgram::read_progdef_hdr( FILE* fp )
{
  BSCRIPT_PROGDEF_HDR hdr;
  if ( fread( &hdr, sizeof hdr, 1, fp ) != 1 )
    return -1;

  haveProgram = true;
  expectedArgs = hdr.expectedArgs;
  return 0;
}

/**
 * Reads a module "usages" section from the file pointer
 */
int EScriptProgram::read_module( FILE* fp )
{
  BSCRIPT_MODULE_HDR hdr;
  if ( fread( &hdr, sizeof hdr, 1, fp ) != 1 )
    return -1;
  auto fm = new FunctionalityModule( hdr.modulename );
  for ( unsigned i = 0; i < hdr.nfuncs; i++ )
  {
    BSCRIPT_MODULE_FUNCTION func;
    if ( fread( &func, sizeof func, 1, fp ) != 1 )
    {
      delete fm;
      return -1;
    }
    fm->addFunction( func.funcname, func.nargs );
  }
  modules.push_back( fm );
  return 0;
}

/* Note: This function is ONLY used from Executor::read(). */
int EScriptProgram::_readToken( Token& token, unsigned position ) const
{
  StoredToken st;
  tokens.atGet1( position, st );
  // StoredToken& st = tokens[position];

  token.module = (ModuleID)st.module;
  token.id = static_cast<BTokenId>( st.id );
  token.type = static_cast<BTokenType>( st.type );
  token.lval = st.offset;

  token.nulStr();

  // FIXME: USED to set lval to 0 for TYP_FUNC.  Not sure if needed anymore.
  // FIXME: Doesn't seem to be, but must be sure before removal
  switch ( st.id )
  {
  case INS_CASEJMP:
    if ( st.offset >= symbols.length() )
    {
      throw std::runtime_error(
          "Symbol offset of " + Clib::tostring( st.offset ) + " exceeds symbol store length of " +
          Clib::tostring( symbols.length() ) + " at PC=" + Clib::tostring( position ) );
    }
    token.dataptr = reinterpret_cast<const unsigned char*>( symbols.array() + st.offset );
    return 0;
  case TOK_LONG:
    if ( st.offset >= symbols.length() )
    {
      throw std::runtime_error(
          "Symbol offset of " + Clib::tostring( st.offset ) + " exceeds symbol store length of " +
          Clib::tostring( symbols.length() ) + " at PC=" + Clib::tostring( position ) );
    }
    std::memcpy( &token.lval, symbols.array() + st.offset, sizeof( int ) );
    return 0;
  case TOK_DOUBLE:
    if ( st.offset >= symbols.length() )
    {
      throw std::runtime_error(
          "Symbol offset of " + Clib::tostring( st.offset ) + " exceeds symbol store length of " +
          Clib::tostring( symbols.length() ) + " at PC=" + Clib::tostring( position ) );
    }
    std::memcpy( &token.dval, symbols.array() + st.offset, sizeof( double ) );
    return 0;

  case CTRL_STATEMENTBEGIN:
    if ( st.offset )
    {
      if ( st.offset >= symbols.length() )
      {
        throw std::runtime_error(
            "Symbol offset of " + Clib::tostring( st.offset ) + " exceeds symbol store length of " +
            Clib::tostring( symbols.length() ) + " at PC=" + Clib::tostring( position ) );
      }
      DebugToken* dt = (DebugToken*)( symbols.array() + st.offset );
      token.sourceFile = dt->sourceFile;
      token.lval = dt->offset;

      if ( dt->strOffset >= symbols.length() )
      {
        throw std::runtime_error( "Symbol offset of " + Clib::tostring( dt->strOffset ) +
                                  " exceeds symbol store length of " +
                                  Clib::tostring( symbols.length() ) +
                                  " at PC=" + Clib::tostring( position ) );
      }
      if ( dt->strOffset )
        token.setStr( symbols.array() + dt->strOffset );
    }
    return 0;

  case INS_INITFOREACH:
  case INS_STEPFOREACH:
  case INS_INITFOR:
  case INS_NEXTFOR:
  case TOK_GLOBALVAR:
  case TOK_LOCALVAR:
  case CTRL_JSR_USERFUNC:
  case CTRL_LEAVE_BLOCK:
  case TOK_ARRAY_SUBSCRIPT:
  case RSV_JMPIFFALSE:
  case RSV_JMPIFTRUE:
  case RSV_GOTO:
  case RSV_LOCAL:
  case RSV_GLOBAL:
  case INS_ASSIGN_GLOBALVAR:
  case INS_ASSIGN_LOCALVAR:
  case INS_GET_MEMBER_ID:
  case INS_SET_MEMBER_ID:
  case INS_SET_MEMBER_ID_CONSUME:
  case INS_CALL_METHOD_ID:
  case INS_SET_MEMBER_ID_CONSUME_PLUSEQUAL:
  case INS_SET_MEMBER_ID_CONSUME_MINUSEQUAL:
  case INS_SET_MEMBER_ID_CONSUME_TIMESEQUAL:
  case INS_SET_MEMBER_ID_CONSUME_DIVIDEEQUAL:
  case INS_SET_MEMBER_ID_CONSUME_MODULUSEQUAL:
  case INS_SET_MEMBER_ID_UNPLUSPLUS:
  case INS_SET_MEMBER_ID_UNMINUSMINUS:
  case INS_SET_MEMBER_ID_UNPLUSPLUS_POST:
  case INS_SET_MEMBER_ID_UNMINUSMINUS_POST:
    token.lval = st.offset;
    return 0;
  case TOK_FUNCREF:
    token.lval = st.offset;
    return 0;

  case INS_CALL_METHOD:
  case TOK_FUNC:
  case TOK_USERFUNC:
    token.lval = st.type;
    token.type = ( st.id == TOK_FUNC ) ? TYP_FUNC : TYP_USERFUNC;
    if ( st.offset )
    {
      if ( st.offset >= symbols.length() )
      {
        throw std::runtime_error(
            "Symbol offset of " + Clib::tostring( st.offset ) + " exceeds symbol store length of " +
            Clib::tostring( symbols.length() ) + " at PC=" + Clib::tostring( position ) );
      }
      token.setStr( symbols.array() + st.offset );
    }
    else if ( token.type == TYP_FUNC )
    {
      FunctionalityModule* modl = modules[token.module];
      const ModuleFunction* modfunc;

      // executor only:
      modfunc = modl->functions.at( token.lval );

      token.setStr( modfunc->name.get().c_str() );
    }
    return 0;

  default:
    if ( st.offset )
    {
      if ( st.offset >= symbols.length() )
      {
        throw std::runtime_error(
            "Symbol offset of " + Clib::tostring( st.offset ) + " exceeds symbol store length of " +
            Clib::tostring( symbols.length() ) + " at PC=" + Clib::tostring( position ) );
      }
      token.setStr( symbols.array() + st.offset );
    }
    return 0;
  }
}

int EScriptProgram::read_globalvarnames( FILE* fp )
{
  BSCRIPT_GLOBALVARNAMES_HDR hdr;
  if ( fread( &hdr, sizeof hdr, 1, fp ) != 1 )
    return -1;
  int res = 0;
  unsigned bufalloc = 20;
  auto buffer = new char[bufalloc];
  for ( unsigned idx = 0; idx < hdr.nGlobalVars; ++idx )
  {
    BSCRIPT_GLOBALVARNAME_HDR ghdr;
    if ( fread( &ghdr, sizeof ghdr, 1, fp ) != 1 )
    {
      res = -1;
      break;
    }
    if ( ghdr.namelen >= bufalloc )
    {
      bufalloc = ghdr.namelen + 5;
      delete[] buffer;
      buffer = new char[bufalloc];
    }
    if ( fread( buffer, ghdr.namelen + 1, 1, fp ) != 1 )
    {
      res = -1;
      break;
    }
    globalvarnames.push_back( std::string( buffer ) );
  }
  delete[] buffer;
  buffer = nullptr;
  return res;
}

int EScriptProgram::read_exported_functions( FILE* fp, BSCRIPT_SECTION_HDR* hdr )
{
  BSCRIPT_EXPORTED_FUNCTION bef;
  ObjMethod* mth;

  unsigned nexports = hdr->length / sizeof bef;
  while ( nexports-- )
  {
    if ( fread( &bef, sizeof bef, 1, fp ) != 1 )
      return -1;
    EPExportedFunction ef;
    ef.name = bef.funcname;
    ef.nargs = bef.nargs;
    ef.PC = bef.PC;
    exported_functions.push_back( ef );
    if ( ( mth = getKnownObjMethod( ef.name.c_str() ) ) != nullptr )
      mth->overridden = true;
  }
  return 0;
}

int EScriptProgram::read_dbg_file()
{
  if ( debug_loaded )
    return 0;

  std::string mname = name;
  mname.replace( mname.size() - 3, 3, "dbg" );
  FILE* fp = fopen( mname.c_str(), "rb" );
  if ( !fp )
  {
    ERROR_PRINT << "Unable to open " << mname << "\n";
    return -1;
  }

  u32 dbgversion;
  size_t fread_res = fread( &dbgversion, sizeof dbgversion, 1, fp );
  if ( fread_res != 1 || ( dbgversion != 2 && dbgversion != 3 ) )
  {
    ERROR_PRINT << "Recompile required. Bad version " << dbgversion << " in " << mname
                << ", expected version 2\n";
    fclose( fp );
    return -1;
  }

  size_t bufalloc = 20;
  auto buffer = std::unique_ptr<char[]>( new char[bufalloc] );
  int res = 0;

  u32 count;
  fread_res = fread( &count, sizeof count, 1, fp );
  if ( fread_res != 1 )
  {
    fclose( fp );
    return -1;
  }
  dbg_filenames.resize( count );
  for ( auto& elem : dbg_filenames )
  {
    fread_res = fread( &count, sizeof count, 1, fp );
    if ( fread_res != 1 )
    {
      fclose( fp );
      return -1;
    }
    if ( count >= bufalloc )
    {
      bufalloc = count * 2;
      buffer.reset( new char[bufalloc] );
    }
    fread_res = fread( buffer.get(), count, 1, fp );
    if ( fread_res != 1 )
    {
      fclose( fp );
      return -1;
    }
    elem = buffer.get();
  }

  fread_res = fread( &count, sizeof count, 1, fp );
  if ( fread_res != 1 )
  {
    fclose( fp );
    return -1;
  }
  globalvarnames.resize( count );
  for ( auto& elem : globalvarnames )
  {
    fread_res = fread( &count, sizeof count, 1, fp );
    if ( fread_res != 1 )
    {
      fclose( fp );
      return -1;
    }
    if ( count >= bufalloc )
    {
      bufalloc = count * 2;
      buffer.reset( new char[bufalloc] );
    }
    fread_res = fread( buffer.get(), count, 1, fp );
    if ( fread_res != 1 )
    {
      fclose( fp );
      return -1;
    }
    elem = buffer.get();
  }

  fread_res = fread( &count, sizeof count, 1, fp );
  if ( fread_res != 1 )
  {
    fclose( fp );
    return -1;
  }
  dbg_filenum.resize( count );
  dbg_linenum.resize( count );
  dbg_ins_blocks.resize( count );
  dbg_ins_statementbegin.resize( count );
  for ( unsigned i = 0; i < tokens.count(); ++i )
  {
    BSCRIPT_DBG_INSTRUCTION ins;
    fread_res = fread( &ins, sizeof ins, 1, fp );
    if ( fread_res != 1 )
    {
      fclose( fp );
      return -1;
    }
    dbg_filenum[i] = ins.filenum;
    dbg_linenum[i] = ins.linenum;
    dbg_ins_blocks[i] = ins.blocknum;
    dbg_ins_statementbegin[i] = ins.statementbegin ? true : false;
  }
  fread_res = fread( &count, sizeof count, 1, fp );
  if ( fread_res != 1 )
  {
    fclose( fp );
    return -1;
  }
  blocks.resize( count );
  for ( auto& block : blocks )
  {
    u32 tmp;

    fread_res = fread( &tmp, sizeof tmp, 1, fp );
    if ( fread_res != 1 )
    {
      fclose( fp );
      return -1;
    }
    block.parentblockidx = tmp;

    fread_res = fread( &tmp, sizeof tmp, 1, fp );
    if ( fread_res != 1 )
    {
      fclose( fp );
      return -1;
    }
    block.parentvariables = tmp;

    fread_res = fread( &tmp, sizeof tmp, 1, fp );
    if ( fread_res != 1 )
    {
      fclose( fp );
      return -1;
    }
    block.localvarnames.resize( tmp );

    for ( auto& elem : block.localvarnames )
    {
      fread_res = fread( &count, sizeof count, 1, fp );
      if ( fread_res != 1 )
      {
        fclose( fp );
        return -1;
      }
      if ( count >= bufalloc )
      {
        bufalloc = count * 2;
        buffer.reset( new char[bufalloc] );
      }
      fread_res = fread( buffer.get(), count, 1, fp );
      if ( fread_res != 1 )
      {
        fclose( fp );
        return -1;
      }
      elem = buffer.get();
    }
  }
  if ( dbgversion >= 3 )
  {
    fread_res = fread( &count, sizeof count, 1, fp );
    if ( fread_res != 1 )
    {
      fclose( fp );
      return -1;
    }
    dbg_functions.resize( count );
    for ( auto& func : dbg_functions )
    {
      u32 tmp;
      fread_res = fread( &tmp, sizeof tmp, 1, fp );
      if ( fread_res != 1 )
      {
        fclose( fp );
        return -1;
      }
      if ( tmp >= bufalloc )
      {
        bufalloc = tmp * 2;
        buffer.reset( new char[bufalloc] );
      }
      fread_res = fread( buffer.get(), tmp, 1, fp );
      if ( fread_res != 1 )
      {
        fclose( fp );
        return -1;
      }
      func.name = buffer.get();

      fread_res = fread( &tmp, sizeof tmp, 1, fp );
      if ( fread_res != 1 )
      {
        fclose( fp );
        return -1;
      }
      func.firstPC = tmp;
      fread_res = fread( &tmp, sizeof tmp, 1, fp );
      if ( fread_res != 1 )
      {
        fclose( fp );
        return -1;
      }
      func.lastPC = tmp;
    }
  }

  fclose( fp );
  debug_loaded = true;
  return res;
}
}  // namespace Bscript
}  // namespace Pol
