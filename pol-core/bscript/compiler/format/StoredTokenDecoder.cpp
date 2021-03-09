#include "StoredTokenDecoder.h"

#include "StoredToken.h"
#include "bscript/compiler/representation/DebugStore.h"
#include "bscript/compiler/representation/ModuleDescriptor.h"
#include "bscript/compiler/representation/ModuleFunctionDescriptor.h"
#include "objmembers.h"
#include "objmethods.h"

namespace Pol::Bscript::Compiler
{
StoredTokenDecoder::StoredTokenDecoder( const std::vector<ModuleDescriptor>& module_descriptors,
                                        const std::vector<std::byte>& data )
  : module_descriptors( module_descriptors ), data( data )
{
}

void StoredTokenDecoder::decode_to( const StoredToken& tkn, fmt::Writer& w )
{
  switch ( tkn.id )
  {
  case TOK_LONG:
    w << integer_at( tkn.offset ) << " (integer)"
      << " offset=0x" << fmt::hex( tkn.offset );
    break;
  case TOK_DOUBLE:
    w << double_at( tkn.offset ) << " (float)"
      << " offset=0x" << fmt::hex( tkn.offset );
    break;
  case TOK_STRING:
  {
    auto s = string_at( tkn.offset );
    w << Clib::getencodedquotedstring( s ) << " (string)"
      << " len=" << s.length() << " offset=0x" << fmt::hex( tkn.offset );
    break;
  }

  case TOK_ADD:
    w << "+";
    break;
  case TOK_SUBTRACT:
    w << "-";
    break;
  case TOK_MULT:
    w << "*";
    break;
  case TOK_DIV:
    w << "/";
    break;

  case TOK_ASSIGN:
    w << ":=";
    break;
  case INS_ASSIGN_CONSUME:
    w << ":= #";
    break;

  case TOK_PLUSEQUAL:
    w << "+=";
    break;
  case TOK_MINUSEQUAL:
    w << "-=";
    break;
  case TOK_TIMESEQUAL:
    w << "*=";
    break;
  case TOK_DIVIDEEQUAL:
    w << "/=";
    break;
  case TOK_MODULUSEQUAL:
    w << "%=";
    break;
  case TOK_INSERTINTO:
    w << "append-array-element (TOK_INSERTINTO)";
    break;

  case TOK_LESSTHAN:
    w << "<";
    break;
  case TOK_LESSEQ:
    w << "<=";
    break;
  case TOK_GRTHAN:
    w << ">";
    break;
  case TOK_GREQ:
    w << ">=";
    break;

  case TOK_AND:
    w << "&& (logical and)";
    break;
  case TOK_OR:
    w << "|| (logical or)";
    break;

    // equalite/inequality operators */
  case TOK_EQUAL:
    w << "==";
    break;
  case TOK_NEQ:
    w << "!=";
    break;

  case TOK_UNPLUS:
    w << "unary +";
    break;
  case TOK_UNMINUS:
    w << "unary -";
    break;
  case TOK_LOG_NOT:
    w << "! (logical inversion)";
    break;
  case TOK_BITWISE_NOT:
    w << "~ (bitwise inversion)";
    break;

  case TOK_CONSUMER:
    w << "# (consume)";
    break;

  case TOK_ARRAY_SUBSCRIPT:
    w << "TOK_ARRAY_SUBSCRIPT";
    break;

  case TOK_ADDMEMBER:
    w << ".+ (add-member)";
    break;
  case TOK_DELMEMBER:
    w << ".- (delete-member)";
    break;
  case TOK_CHKMEMBER:
    w << ".? (check-member)";
    break;

  case CTRL_STATEMENTBEGIN:
  {
    const Pol::Bscript::DebugToken& debug_token = debug_token_at( tkn.offset );

    w << "CTRL_STATEMENTBEGIN"
      << " sourceFile=" << debug_token.sourceFile << " x=" << debug_token.offset
      << " y=" << string_at( debug_token.strOffset );
    break;
  }
  case CTRL_PROGEND:
    w << "progend";
    break;
  case CTRL_MAKELOCAL:
    w << "makelocal";
    break;
  case CTRL_JSR_USERFUNC:
    w << "jsr userfunc @" << tkn.offset;
    break;
  case INS_POP_PARAM:
    w << "pop param '" << string_at( tkn.offset ) << "'";
    break;
  case CTRL_LEAVE_BLOCK:
    w << "leave block (remove " + std::to_string( tkn.offset ) + " locals)";
    break;

  case RSV_JMPIFFALSE:
    w << "if false goto " + std::to_string( tkn.offset );
    break;
  case RSV_JMPIFTRUE:
    w << "if true goto " + std::to_string( tkn.offset );
    break;
  case RSV_GOTO:
    w << "goto " << tkn.offset;
    break;
  case RSV_RETURN:
    w << "return";
    break;
  case RSV_EXIT:
    w << "exit";
    break;

  case RSV_LOCAL:
    w << "declare local #" << tkn.offset;
    break;
  case RSV_GLOBAL:
    w << "declare global #" << tkn.offset;
    break;

  case INS_DECLARE_ARRAY:
    w << "declare array";
    break;

  case TOK_FUNC:
  {
    unsigned module_id = tkn.module;
    unsigned function_index = tkn.type;
    w << "call module function (" << module_id << ", " << function_index << "): ";
    if ( module_id >= module_descriptors.size() )
    {
      w << "module index " << module_id << " exceeds module_descriptors size "
        << module_descriptors.size();
      break;
    }
    auto& module = module_descriptors.at( module_id );
    if ( function_index >= module.functions.size() )
    {
      w << "function index " << function_index << " exceeds module.functions size "
        << module.functions.size();
      break;
    }
    auto& defn = module.functions.at( function_index );
    w << defn.name;
    break;
  }
  case TOK_ERROR:
    w << "error";
    break;

  case TOK_LOCALVAR:
    w << "local variable #" << tkn.offset;
    break;
  case TOK_GLOBALVAR:
    w << "global variable #" << tkn.offset;
    break;

  case INS_INITFOREACH:
    w << "initforeach @" << tkn.offset;
    break;
  case INS_STEPFOREACH:
    w << "stepforeach @" << tkn.offset;
    break;
  case INS_CASEJMP:
  {
    w << "casejmp";
    decode_casejmp_table( w, tkn.offset );
    break;
  }
  case INS_GET_ARG:
    w << "get arg '" << string_at( tkn.offset ) << "'";
    break;
  case TOK_ARRAY:
    w << "create-array (TOK_ARRAY)";
    break;

  case INS_CALL_METHOD:
    w << "call method '" << string_at( tkn.offset ) << "'";
    break;

  case TOK_DICTIONARY:
    w << "TOK_DICTIONARY";
    break;
  case INS_INITFOR:
    w << "initfor @" << tkn.offset;
    break;
  case INS_NEXTFOR:
    w << "nextfor @" << tkn.offset;
    break;
  case INS_POP_PARAM_BYREF:
    w << "pop param byref '" << string_at( tkn.offset ) << "'";
    break;
  case TOK_MODULUS:
    w << "modulus";
    break;

  case TOK_BSLEFT:
    w << "<<";  // bitshift-left-sift";
    break;
  case TOK_BSRIGHT:
    w << ">>";  // bitshift-right-shift";
    break;
  case TOK_BITAND:
    w << "&";  // bitwise-and";
    break;
  case TOK_BITOR:
    w << "|";  // bitwise-or";
    break;
  case TOK_BITXOR:
    w << "^";  // bitwise-xor";
    break;

  case TOK_STRUCT:
    w << "create empty struct";
    break;
  case INS_SUBSCRIPT_ASSIGN:
    w << "subscript assign";
    break;
  case INS_SUBSCRIPT_ASSIGN_CONSUME:
    w << "subscript assign and consume";
    break;
  case INS_MULTISUBSCRIPT:
    w << "multi subscript get (" << tkn.offset << " indexes)";
    break;
  case INS_MULTISUBSCRIPT_ASSIGN:
    w << "multi subscript assign (" << tkn.offset << " indexes)";
    break;
  case INS_ASSIGN_LOCALVAR:
    w << "local #" << tkn.offset;
    w << " :=";
    break;
  case INS_ASSIGN_GLOBALVAR:
    w << "global #" << tkn.offset;
    w << " :=";
    break;

  case INS_GET_MEMBER:
    w << "get-member " << string_at( tkn.offset ) << "(offset " << tkn.offset << ")";
    break;
  case INS_SET_MEMBER:
    w << "set-member " << string_at( tkn.offset ) << "(offset " << tkn.offset << ")";
    break;
  case INS_SET_MEMBER_CONSUME:
    w << "set-member-consume " << string_at( tkn.offset ) << "(offset " << tkn.offset << ")";
    break;

  case INS_ADDMEMBER2:
    w << "add uninitialized member (" << string_at( tkn.offset ) << " to struct";
    break;
  case INS_ADDMEMBER_ASSIGN:
    w << "add member (" << string_at( tkn.offset ) << " to struct";
    break;
  case INS_UNINIT:
    w << "push-uninit";
    break;
  case INS_DICTIONARY_ADDMEMBER:
    w << "add member to dictionary";
    break;

  case INS_GET_MEMBER_ID:
    w << "get-member-id '" << getObjMember( tkn.type )->code << "' (" << tkn.type << ")";
    break;
  case INS_SET_MEMBER_ID:
    w << "set-member-id '" << getObjMember( tkn.type )->code << "' (" << tkn.type << ")";
    break;
  case INS_SET_MEMBER_ID_CONSUME:
    w << "set-member-id-consume '" << getObjMember( tkn.type )->code << "' (" << tkn.type << ")";
    break;
  case INS_CALL_METHOD_ID:
    w << "call-method-id '" << getObjMethod( tkn.offset )->code << "' (#" << tkn.offset << ", "
      << tkn.type << " arguments)";
    break;

  case INS_SET_MEMBER_ID_CONSUME_PLUSEQUAL:
    w << "set member id '" << getObjMember( tkn.offset )->code << "' (" << tkn.offset << ")  += #";
    break;
  case INS_SET_MEMBER_ID_CONSUME_MINUSEQUAL:
    w << "set member id '" << getObjMember( tkn.offset )->code << "' (" << tkn.offset << ")  -= #";
    break;
  case INS_SET_MEMBER_ID_CONSUME_TIMESEQUAL:
    w << "set member id '" << getObjMember( tkn.offset )->code << "' (" << tkn.offset << ")  *= #";
    break;
  case INS_SET_MEMBER_ID_CONSUME_DIVIDEEQUAL:
    w << "set member id '" << getObjMember( tkn.offset )->code << "' (" << tkn.offset << ")  /= #";
    break;
  case INS_SET_MEMBER_ID_CONSUME_MODULUSEQUAL:
    w << "set member id '" << getObjMember( tkn.offset )->code << "' (" << tkn.offset << ")  %= #";
    break;

  case TOK_FUNCREF:
  {
    w << "Function Ref "
      << "--function name not available--"
      << " @" << tkn.offset;
    break;
  }

  case TOK_UNPLUSPLUS:
    w << "prefix unary ++";
    break;
  case TOK_UNMINUSMINUS:
    w << "prefix unary --";
    break;
  case TOK_UNPLUSPLUS_POST:
    w << "postfix unary ++";
    break;
  case TOK_UNMINUSMINUS_POST:
    w << "postfix unary --";
    break;
  case INS_SET_MEMBER_ID_UNPLUSPLUS:
    w << "set member id '" << getObjMember( tkn.offset )->code << "' unary ++";
    break;
  case INS_SET_MEMBER_ID_UNMINUSMINUS:
    w << "set member id '" << getObjMember( tkn.offset )->code << "' unary --";
    break;
  case INS_SET_MEMBER_ID_UNPLUSPLUS_POST:
    w << "set member id '" << getObjMember( tkn.offset )->code << "' unary ++ post";
    break;
  case INS_SET_MEMBER_ID_UNMINUSMINUS_POST:
    w << "set member id '" << getObjMember( tkn.offset )->code << "' unary -- post";
    break;
  case INS_SKIPIFTRUE_ELSE_CONSUME:
    w << "peek at top of stack; skip " << tkn.offset
      << " instructions if true, otherwise consume it";
    break;
  case TOK_INTERPOLATED_STRING:
    w << "interpolated string join (" << tkn.offset << " parts)";
    break;
  case TOK_FORMAT_EXPRESSION:
    w << "format expression";
    break;

  default:
    w << "id=0x" << fmt::hex( tkn.id ) << " type=" << tkn.type << " offset=" << tkn.offset
      << " module=" << tkn.module;
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

void StoredTokenDecoder::decode_casejmp_table( fmt::Writer& w, unsigned offset ) const
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
      w << "\n" << indent << value << ": @" << jump_address;
    }
    else if ( type == CASE_TYPE_DEFAULT )
    {
      w << "\n" << indent << "default: @" << jump_address;
      break;
    }
    else
    {
      unsigned string_length = type;  // type is the length of the string, otherwise
      if ( offset + string_length > data.size() )
        throw std::runtime_error( "casejmp string at offset " + std::to_string( offset ) + " of " +
                                  std::to_string( string_length ) +
                                  " characters exceeds data size of " +
                                  std::to_string( data.size() ) );

      const char* s_begin = reinterpret_cast<const char*>( data.data() + offset );
      std::string contents( s_begin, s_begin + string_length );
      w << "\n" << indent << Clib::getencodedquotedstring( contents ) << ": @" << jump_address;
      offset += string_length;
    }
  }
}

}  // namespace Pol::Bscript::Compiler
