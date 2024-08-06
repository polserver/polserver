#include "StoredTokenDecoder.h"

#include "StoredToken.h"
#include "bscript/compiler/representation/DebugStore.h"
#include "bscript/compiler/representation/ModuleDescriptor.h"
#include "bscript/compiler/representation/ModuleFunctionDescriptor.h"
#include "objmembers.h"
#include "objmethods.h"
#include <algorithm>
#include <iterator>

namespace Pol::Bscript::Compiler
{
StoredTokenDecoder::StoredTokenDecoder( const std::vector<ModuleDescriptor>& module_descriptors,
                                        const std::vector<std::byte>& data )
    : module_descriptors( module_descriptors ), data( data )
{
}

void StoredTokenDecoder::decode_to( const StoredToken& tkn, std::string& w )
{
  switch ( tkn.id )
  {
  case TOK_LONG:
    fmt::format_to( std::back_inserter( w ), "{} (integer) offset={:#x}", integer_at( tkn.offset ),
                    tkn.offset );
    break;
  case TOK_DOUBLE:
    fmt::format_to( std::back_inserter( w ), "{} (float) offset={:#x}", double_at( tkn.offset ),
                    tkn.offset );
    break;
  case TOK_STRING:
  {
    auto s = string_at( tkn.offset );
    fmt::format_to( std::back_inserter( w ), "{} (string) len={} offset={:#x}",
                    Clib::getencodedquotedstring( s ), s.length(), tkn.offset );
    break;
  }
  case TOK_BOOL:
    fmt::format_to( std::back_inserter( w ), "{} (boolean)", static_cast<bool>( tkn.offset ) );
    break;
  case TOK_ADD:
    w += "+";
    break;
  case TOK_SUBTRACT:
    w += "-";
    break;
  case TOK_MULT:
    w += "*";
    break;
  case TOK_DIV:
    w += "/";
    break;

  case TOK_ASSIGN:
    w += ":=";
    break;
  case INS_ASSIGN_CONSUME:
    w += ":= #";
    break;

  case TOK_PLUSEQUAL:
    w += "+=";
    break;
  case TOK_MINUSEQUAL:
    w += "-=";
    break;
  case TOK_TIMESEQUAL:
    w += "*=";
    break;
  case TOK_DIVIDEEQUAL:
    w += "/=";
    break;
  case TOK_MODULUSEQUAL:
    w += "%=";
    break;
  case TOK_INSERTINTO:
    w += "append-array-element (TOK_INSERTINTO)";
    break;

  case TOK_LESSTHAN:
    w += "<";
    break;
  case TOK_LESSEQ:
    w += "<=";
    break;
  case TOK_GRTHAN:
    w += ">";
    break;
  case TOK_GREQ:
    w += ">=";
    break;

  case TOK_AND:
    w += "&& (logical and)";
    break;
  case TOK_OR:
    w += "|| (logical or)";
    break;

    // equalite/inequality operators */
  case TOK_EQUAL:
    w += "==";
    break;
  case TOK_NEQ:
    w += "!=";
    break;

  case TOK_UNPLUS:
    w += "unary +";
    break;
  case TOK_UNMINUS:
    w += "unary -";
    break;
  case TOK_LOG_NOT:
    w += "! (logical inversion)";
    break;
  case TOK_BITWISE_NOT:
    w += "~ (bitwise inversion)";
    break;

  case TOK_CONSUMER:
    w += "# (consume)";
    break;

  case TOK_ARRAY_SUBSCRIPT:
    w += "TOK_ARRAY_SUBSCRIPT";
    break;

  case TOK_ADDMEMBER:
    w += ".+ (add-member)";
    break;
  case TOK_DELMEMBER:
    w += ".- (delete-member)";
    break;
  case TOK_CHKMEMBER:
    w += ".? (check-member)";
    break;

  case CTRL_STATEMENTBEGIN:
  {
    const Pol::Bscript::DebugToken& debug_token = debug_token_at( tkn.offset );

    fmt::format_to( std::back_inserter( w ), "CTRL_STATEMENTBEGIN sourceFile={} x={} y={}",
                    debug_token.sourceFile, debug_token.offset,
                    string_at( debug_token.strOffset ) );
    break;
  }
  case CTRL_PROGEND:
    w += "progend";
    break;
  case CTRL_MAKELOCAL:
    w += "makelocal";
    break;
  case CTRL_JSR_USERFUNC:
    fmt::format_to( std::back_inserter( w ), "jsr userfunc @{}", tkn.offset );
    break;
  case INS_POP_PARAM:
    fmt::format_to( std::back_inserter( w ), "pop param '{}'", string_at( tkn.offset ) );
    break;
  case CTRL_LEAVE_BLOCK:
    fmt::format_to( std::back_inserter( w ), "leave block (remove {} locals)", tkn.offset );
    break;

  case RSV_JMPIFFALSE:
    fmt::format_to( std::back_inserter( w ), "if false goto {}", tkn.offset );
    break;
  case RSV_JMPIFTRUE:
    fmt::format_to( std::back_inserter( w ), "if true goto {}", tkn.offset );
    break;
  case RSV_GOTO:
    fmt::format_to( std::back_inserter( w ), "goto {}", tkn.offset );
    break;
  case RSV_RETURN:
    w += "return";
    break;
  case RSV_EXIT:
    w += "exit";
    break;

  case RSV_LOCAL:
    fmt::format_to( std::back_inserter( w ), "declare local #{}", tkn.offset );
    break;
  case RSV_GLOBAL:
    fmt::format_to( std::back_inserter( w ), "declare global #{}", tkn.offset );
    break;

  case INS_DECLARE_ARRAY:
    w += "declare array";
    break;

  case TOK_FUNC:
  {
    unsigned module_id = tkn.module;
    unsigned function_index = tkn.type;
    fmt::format_to( std::back_inserter( w ), "call module function ({}, {}): ", module_id,
                    function_index );
    if ( module_id >= module_descriptors.size() )
    {
      fmt::format_to( std::back_inserter( w ), "module index {} exceeds module_descriptors size {}",
                      module_id, module_descriptors.size() );
      break;
    }
    auto& module = module_descriptors.at( module_id );
    if ( function_index >= module.functions.size() )
    {
      fmt::format_to( std::back_inserter( w ), "function index {} exceeds module.functions size {}",
                      function_index, module.functions.size() );
      break;
    }
    auto& defn = module.functions.at( function_index );
    fmt::format_to( std::back_inserter( w ), "{}", defn.name );
    break;
  }
  case TOK_ERROR:
    w += "error";
    break;

  case TOK_LOCALVAR:
    fmt::format_to( std::back_inserter( w ), "local variable #{}", tkn.offset );
    break;
  case TOK_GLOBALVAR:
    fmt::format_to( std::back_inserter( w ), "global variable #{}", tkn.offset );
    break;

  case INS_INITFOREACH:
    fmt::format_to( std::back_inserter( w ), "initforeach @{}", tkn.offset );
    break;
  case INS_STEPFOREACH:
    fmt::format_to( std::back_inserter( w ), "stepforeach @{}", tkn.offset );
    break;
  case INS_CASEJMP:
  {
    w += "casejmp";
    decode_casejmp_table( w, tkn.offset );
    break;
  }
  case INS_GET_ARG:
    fmt::format_to( std::back_inserter( w ), "get arg '{}'", string_at( tkn.offset ) );
    break;
  case TOK_ARRAY:
    w += "create-array (TOK_ARRAY)";
    break;

  case INS_CALL_METHOD:
    fmt::format_to( std::back_inserter( w ), "call method '{}'", string_at( tkn.offset ) );
    break;

  case TOK_DICTIONARY:
    w += "TOK_DICTIONARY";
    break;
  case INS_INITFOR:
    fmt::format_to( std::back_inserter( w ), "initfor @{}", tkn.offset );
    break;
  case INS_NEXTFOR:
    fmt::format_to( std::back_inserter( w ), "nextfor @{}", tkn.offset );
    break;
  case INS_POP_PARAM_BYREF:
    fmt::format_to( std::back_inserter( w ), "pop param byref '{}'", string_at( tkn.offset ) );
    break;
  case TOK_MODULUS:
    w += "modulus";
    break;

  case TOK_BSLEFT:
    w += "<<";  // bitshift-left-sift";
    break;
  case TOK_BSRIGHT:
    w += ">>";  // bitshift-right-shift";
    break;
  case TOK_BITAND:
    w += "&";  // bitwise-and";
    break;
  case TOK_BITOR:
    w += "|";  // bitwise-or";
    break;
  case TOK_BITXOR:
    w += "^";  // bitwise-xor";
    break;

  case TOK_STRUCT:
    w += "create empty struct";
    break;
  case INS_SUBSCRIPT_ASSIGN:
    w += "subscript assign";
    break;
  case INS_SUBSCRIPT_ASSIGN_CONSUME:
    w += "subscript assign and consume";
    break;
  case INS_MULTISUBSCRIPT:
    fmt::format_to( std::back_inserter( w ), "multi subscript get ({} indexes)", tkn.offset );
    break;
  case INS_MULTISUBSCRIPT_ASSIGN:
    fmt::format_to( std::back_inserter( w ), "multi subscript assign ({} indexes)", tkn.offset );
    break;
  case INS_ASSIGN_LOCALVAR:
    fmt::format_to( std::back_inserter( w ), "local #{} :=", tkn.offset );
    break;
  case INS_ASSIGN_GLOBALVAR:
    fmt::format_to( std::back_inserter( w ), "global #{} :=", tkn.offset );
    break;

  case INS_GET_MEMBER:
    fmt::format_to( std::back_inserter( w ), "get-member {} (offset {})", string_at( tkn.offset ),
                    tkn.offset );
    break;
  case INS_SET_MEMBER:
    fmt::format_to( std::back_inserter( w ), "set-member {} (offset {})", string_at( tkn.offset ),
                    tkn.offset );
    break;
  case INS_SET_MEMBER_CONSUME:
    fmt::format_to( std::back_inserter( w ), "set-member-consume {} (offset {})",
                    string_at( tkn.offset ), tkn.offset );
    break;

  case INS_ADDMEMBER2:
    fmt::format_to( std::back_inserter( w ), "add uninitialized member ({}) to struct",
                    string_at( tkn.offset ) );
    break;
  case INS_ADDMEMBER_ASSIGN:
    fmt::format_to( std::back_inserter( w ), "add member ({}) to struct", string_at( tkn.offset ) );
    break;
  case INS_UNINIT:
    w += "push-uninit";
    break;
  case INS_DICTIONARY_ADDMEMBER:
    w += "add member to dictionary";
    break;

  case INS_GET_MEMBER_ID:
    fmt::format_to( std::back_inserter( w ), "get-member-id '{}' ({})",
                    getObjMember( tkn.type )->code, tkn.type );
    break;
  case INS_SET_MEMBER_ID:
    fmt::format_to( std::back_inserter( w ), "set-member-id '{}' ({})",
                    getObjMember( tkn.type )->code, tkn.type );
    break;
  case INS_SET_MEMBER_ID_CONSUME:
    fmt::format_to( std::back_inserter( w ), "set-member-id-consume '{}' ({})",
                    getObjMember( tkn.type )->code, tkn.type );
    break;
  case INS_CALL_METHOD_ID:
    fmt::format_to( std::back_inserter( w ), "call-method-id '{}' (#{}, {} arguments)",
                    getObjMethod( tkn.offset )->code, tkn.offset, tkn.type );
    break;

  case INS_SET_MEMBER_ID_CONSUME_PLUSEQUAL:
    fmt::format_to( std::back_inserter( w ), "set member id '{}' ({}) += #",
                    getObjMember( tkn.offset )->code, tkn.offset );
    break;
  case INS_SET_MEMBER_ID_CONSUME_MINUSEQUAL:
    fmt::format_to( std::back_inserter( w ), "set member id '{}' ({}) -=#",
                    getObjMember( tkn.offset )->code, tkn.offset );
    break;
  case INS_SET_MEMBER_ID_CONSUME_TIMESEQUAL:
    fmt::format_to( std::back_inserter( w ), "set member id '{}' ({}) *= #",
                    getObjMember( tkn.offset )->code, tkn.offset );
    break;
  case INS_SET_MEMBER_ID_CONSUME_DIVIDEEQUAL:
    fmt::format_to( std::back_inserter( w ), "set member id '{}' ({}) /= #",
                    getObjMember( tkn.offset )->code, tkn.offset );
    break;
  case INS_SET_MEMBER_ID_CONSUME_MODULUSEQUAL:
    fmt::format_to( std::back_inserter( w ), "set member id '{}' ({}) %= #",
                    getObjMember( tkn.offset )->code, tkn.offset );
    break;

  case TOK_FUNCTOR:
  {
    fmt::format_to( std::back_inserter( w ),
                    "create-functor index={} instructions={} (TOK_FUNCTOR)",
                    static_cast<int>( tkn.type ), tkn.offset );
    break;
  }

  case TOK_FUNCREF:
  {
    fmt::format_to( std::back_inserter( w ),
                    "create-funcref index={} pc={} (TOK_FUNCREF)",
                    static_cast<int>( tkn.type ), tkn.offset );
    break;
  }

  case TOK_UNPLUSPLUS:
    w += "prefix unary ++";
    break;
  case TOK_UNMINUSMINUS:
    w += "prefix unary --";
    break;
  case TOK_UNPLUSPLUS_POST:
    w += "postfix unary ++";
    break;
  case TOK_UNMINUSMINUS_POST:
    w += "postfix unary --";
    break;
  case INS_SET_MEMBER_ID_UNPLUSPLUS:
    fmt::format_to( std::back_inserter( w ), "set member id '{}' unary ++",
                    getObjMember( tkn.offset )->code );
    break;
  case INS_SET_MEMBER_ID_UNMINUSMINUS:
    fmt::format_to( std::back_inserter( w ), "set member id '{}' unary --",
                    getObjMember( tkn.offset )->code );
    break;
  case INS_SET_MEMBER_ID_UNPLUSPLUS_POST:
    fmt::format_to( std::back_inserter( w ), "set member id '{}' unary ++ post",
                    getObjMember( tkn.offset )->code );
    break;
  case INS_SET_MEMBER_ID_UNMINUSMINUS_POST:
    fmt::format_to( std::back_inserter( w ), "set member id '{}' unary -- post",
                    getObjMember( tkn.offset )->code );
    break;
  case INS_SKIPIFTRUE_ELSE_CONSUME:
    fmt::format_to( std::back_inserter( w ),
                    "peek at top of stack; skip {}"
                    " instructions if true, otherwise consume it",
                    tkn.offset );
    break;
  case TOK_INTERPOLATE_STRING:
    fmt::format_to( std::back_inserter( w ), "interpolate string ({} parts)", tkn.offset );
    break;
  case TOK_FORMAT_EXPRESSION:
    w += "format expression";
    break;

  default:
    fmt::format_to( std::back_inserter( w ), "id={:#x} type={} offset={} module={}", tkn.id,
                    tkn.type, tkn.offset, tkn.module );
  }
}

double StoredTokenDecoder::double_at( unsigned offset ) const
{
  if ( offset > data.size() - sizeof( double ) )
    throw std::runtime_error( "data overflow reading double at offset " + std::to_string( offset ) +
                              ".  Data size is " + std::to_string( data.size() ) );

  return *reinterpret_cast<const double*>( &data[offset] );
}

int StoredTokenDecoder::integer_at( unsigned offset ) const
{
  if ( offset > data.size() - sizeof( int ) )
    throw std::runtime_error( "data overflow reading integer at offset " +
                              std::to_string( offset ) + ".  Data size is " +
                              std::to_string( data.size() ) );

  return *reinterpret_cast<const int*>( &data[offset] );
}

uint16_t StoredTokenDecoder::uint16_t_at( unsigned offset ) const
{
  if ( offset > data.size() - sizeof( uint16_t ) )
    throw std::runtime_error( "data overflow reading uint16_t at offset " +
                              std::to_string( offset ) + ".  Data size is " +
                              std::to_string( data.size() ) );

  return *reinterpret_cast<const uint16_t*>( &data[offset] );
}

std::string StoredTokenDecoder::string_at( unsigned offset ) const
{
  if ( offset > data.size() )
    throw std::runtime_error( "data overflow reading string starting at offset " +
                              std::to_string( offset ) + ".  Data size is " +
                              std::to_string( data.size() ) );

  const char* s_begin = reinterpret_cast<const char*>( data.data() + offset );
  const char* data_end = reinterpret_cast<const char*>( data.data() + data.size() );

  auto s_end = std::find( s_begin, data_end, '\0' );
  if ( s_end == data_end )
    throw std::runtime_error( "No null terminator for string at offset " +
                              std::to_string( offset ) );

  return std::string( s_begin, s_end );
}

const Pol::Bscript::DebugToken& StoredTokenDecoder::debug_token_at( unsigned offset ) const
{
  if ( offset > data.size() - sizeof( Pol::Bscript::DebugToken ) )
    throw std::runtime_error( "data overflow reading DebugToken at offset " +
                              std::to_string( offset ) + ".  Data size is " +
                              std::to_string( data.size() ) );

  return *reinterpret_cast<const Pol::Bscript::DebugToken*>( &data[offset] );
}

void StoredTokenDecoder::decode_casejmp_table( std::string& w, unsigned offset ) const
{
  std::string indent( 24, ' ' );
  for ( ;; )
  {
    unsigned short jump_address = uint16_t_at( offset );
    offset += 2;
    auto type = static_cast<ESCRIPT_CASE_TYPES>( data.at( offset ) );
    offset++;

    if ( type == CASE_TYPE_LONG )
    {
      int value = integer_at( offset );
      offset += 4;
      fmt::format_to( std::back_inserter( w ), "\n{}{}: @{}", indent, value, jump_address );
    }
    else if ( type == CASE_TYPE_BOOL )
    {
      bool value = static_cast<bool>( data.at( offset ) );
      offset += 1;
      fmt::format_to( std::back_inserter( w ), "\n{}{}: @{}", indent, value, jump_address );
    }
    else if ( type == CASE_TYPE_UNINIT )
    {
      fmt::format_to( std::back_inserter( w ), "\n{}<uninit>: @{}", indent, jump_address );
    }
    else if ( type == CASE_TYPE_DEFAULT )
    {
      fmt::format_to( std::back_inserter( w ), "\n{}default: @{}", indent, jump_address );
      break;
    }
    else if ( type == CASE_TYPE_STRING )
    {
      unsigned string_length = static_cast<unsigned>( data.at( offset ) );
      offset += 1;
      if ( offset + string_length > data.size() )
        throw std::runtime_error( "casejmp string at offset " + std::to_string( offset ) + " of " +
                                  std::to_string( string_length ) +
                                  " characters exceeds data size of " +
                                  std::to_string( data.size() ) );

      const char* s_begin = reinterpret_cast<const char*>( data.data() + offset );
      std::string contents( s_begin, s_begin + string_length );
      fmt::format_to( std::back_inserter( w ), "\n{}{}: @{}", indent,
                      Clib::getencodedquotedstring( contents ), jump_address );
      offset += string_length;
    }
    else
    {
      throw std::runtime_error( "casejmp unhandled type " + std::to_string( type ) );
    }
  }
}

}  // namespace Pol::Bscript::Compiler
