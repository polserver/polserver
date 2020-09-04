#include "StoredTokenDecoder.h"

#include "StoredToken.h"
#include "compiler/representation/ModuleDescriptor.h"
#include "compiler/representation/ModuleFunctionDescriptor.h"

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

  case TOK_ADDMEMBER:
    w << ".+ (add-member)";
    break;
  case TOK_DELMEMBER:
    w << ".- (delete-member)";
    break;
  case TOK_CHKMEMBER:
    w << ".? (check-member)";
    break;

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

  case TOK_LOCALVAR:
    w << "local variable #" << tkn.offset;
    break;
  case TOK_GLOBALVAR:
    w << "global variable #" << tkn.offset;
    break;

  case INS_GET_ARG:
    w << "get arg '" << string_at( tkn.offset ) << "'";
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

  case INS_ASSIGN_LOCALVAR:
    w << "local #" << tkn.offset;
    w << " :=";
    break;
  case INS_ASSIGN_GLOBALVAR:
    w << "global #" << tkn.offset;
    w << " :=";
    break;

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

}  // namespace Pol::Bscript::Compiler
