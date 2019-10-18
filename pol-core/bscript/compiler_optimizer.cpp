/** @file
 *
 * @par History
 */


#include "compiler_optimizer.h"

#include <string>

#include "../clib/logfacility.h"
#include "../clib/stlutil.h"
#include "modules.h"
#include "token.h"
#include "tokens.h"
#include <boost/variant.hpp>
#include <boost/variant/multivisitors.hpp>

namespace Pol
{
namespace Bscript
{
namespace
{
// variant of potential token values, void* is just for errors signaled via nullptr
typedef boost::variant<int, double, bool, std::string, void*> token_variant;
// helpers for enable_if
// arithmetic (int, double, bool) calculation possible
// any_string (std::string) string addition/formating
template <typename T1, typename T2>
struct are_arithmetic_tokens
{
  static constexpr bool const value{
      ( std::is_arithmetic<T1>::value && std::is_arithmetic<T2>::value )};
};
template <typename T1, typename T2>
struct is_any_string
{
  static constexpr bool const value{
      ( std::is_same<T1, std::string>::value || std::is_same<T2, std::string>::value )};
};

struct binaryop_token_visitor : public boost::static_visitor<token_variant>
{
  // both are arithmetics call the calculation method
  template <class T1, class T2>
  typename std::enable_if<are_arithmetic_tokens<T1, T2>::value && !is_any_string<T1, T2>::value,
                          token_variant>::type
  operator()( const T1& v1, const T2& v2 ) const
  {
    return arithmetic_oper( v1, v2 );
  }
  // one of the two tokens is a string do string ops
  template <class T1, class T2>
  typename std::enable_if<is_any_string<T1, T2>::value, token_variant>::type operator()(
      const T1& v1, const T2& v2 ) const
  {
    return string_oper( v1, v2 );
  }
  // the rest mainly needed for the compiler
  template <class T1, class T2>
  typename std::enable_if<!are_arithmetic_tokens<T1, T2>::value && !is_any_string<T1, T2>::value,
                          token_variant>::type
  operator()( const T1&, const T2& ) const
  {
    return token_variant( nullptr );
  }

  BTokenId id;  // externally stored operation between both tokens

  // perform the real calculation between 2 arithmetic tokens
  // no special conversion do type conversion like c++
  template <class T1, class T2>
  token_variant arithmetic_oper( const T1& v1, const T2& v2 ) const
  {
    switch ( id )
    {
    case TOK_ADD:
      return token_variant( v1 + v2 );
    case TOK_SUBTRACT:
      return token_variant( v1 - v2 );
    case TOK_MULT:
      return token_variant( v1 * v2 );
    case TOK_DIV:
      if ( v2 == 0.0 )
        throw std::runtime_error( "Program would divide by zero" );
      return token_variant( v1 / v2 );
    case TOK_EQUAL:
      return token_variant( v1 == v2 );
    case TOK_NEQ:
      return token_variant( v1 != v2 );
    case TOK_LESSTHAN:
      return token_variant( v1 < v2 );
    case TOK_LESSEQ:
      return token_variant( v1 <= v2 );
    case TOK_GRTHAN:
      return token_variant( v1 > v2 );
    case TOK_GREQ:
      return token_variant( v1 >= v2 );
    case TOK_AND:
      return token_variant( v1 && v2 );
    case TOK_OR:
      return token_variant( v1 || v2 );

    default:
      return bitoperand( v1, v2 );
    }
  }

  // specific operations with no double involved
  template <class T1, class T2>
  typename std::enable_if<!std::is_floating_point<T1>::value && !std::is_floating_point<T2>::value,
                          token_variant>::type
  bitoperand( const T1& v1, const T2& v2 ) const
  {
    switch ( id )
    {
    case TOK_BSRIGHT:
      return token_variant( v1 >> v2 );
    case TOK_BSLEFT:
      return token_variant( v1 << v2 );
    case TOK_BITAND:
      return token_variant( v1 & v2 );
    case TOK_BITOR:
      return token_variant( v1 | v2 );
    case TOK_BITXOR:
      return token_variant( v1 ^ v2 );
    default:
      return token_variant( nullptr );
    }
  }
  // operations which are invalid for doubles
  template <class T1, class T2>
  typename std::enable_if<std::is_floating_point<T1>::value || std::is_floating_point<T2>::value,
                          token_variant>::type
  bitoperand( const T1&, const T2& ) const
  {
    return token_variant( nullptr );
  }

  // perform the real calculation between one string and any other tokens
  template <class T1, class T2>
  token_variant string_oper( const T1& v1, const T2& v2 ) const
  {
    switch ( id )
    {
    case TOK_ADD:
      return token_variant( tok_to_string( v1 ) + tok_to_string( v2 ) );
    default:
      return token_variant( nullptr );
    }
  }
  template <class T>
  typename std::enable_if<!std::is_same<T, bool>::value, std::string>::type tok_to_string(
      const T& v ) const
  {
    OSTRINGSTREAM os;
    os << v;
    return OSTRINGSTREAM_STR( os );
  }
  template <class T>
  typename std::enable_if<std::is_same<T, bool>::value, std::string>::type tok_to_string(
      T v ) const
  {
    return std::string( v ? "true" : "false" );
  }
};

// visitor struct to apply unary operator
// not really needed to use a variant, but performance doesnt really matter here and this way it
// behaves like the above binary operation
struct unaryop_token_visitor : public boost::static_visitor<token_variant>
{
  BTokenId id;  // externally stored operation

  template <class T>
  token_variant operator()( T ) const
  {
    return token_variant( nullptr );
  }
  token_variant operator()( int v ) const
  {
    switch ( id )
    {
    case TOK_UNPLUS:
      return token_variant( v );
    case TOK_UNMINUS:
      return token_variant( -v );
    case TOK_LOG_NOT:
      return token_variant( !v );
    case TOK_BITWISE_NOT:
      return token_variant( ~v );
    default:
      return token_variant( nullptr );
    }
  }
  token_variant operator()( double v ) const
  {
    switch ( id )
    {
    case TOK_UNPLUS:
      return token_variant( v );
    case TOK_UNMINUS:
      return token_variant( -v );
    default:
      return token_variant( nullptr );
    }
  }
  token_variant operator()( bool v ) const
  {
    switch ( id )
    {
    case TOK_UNPLUS:
      return token_variant( v );
    case TOK_LOG_NOT:
      return token_variant( !v );
    default:
      return token_variant( nullptr );
    }
  }
  token_variant operator()( const std::string& v ) const
  {
    switch ( id )
    {
    case TOK_UNPLUS:
      return token_variant( v );
    default:
      return token_variant( nullptr );
    }
  }
};
// visitor struct to convert the variant to a new Token
struct variant_to_token_visitor : public boost::static_visitor<Token*>
{
  template <class T>
  Token* operator()( T ) const
  {
    return nullptr;
  }
  Token* operator()( int v ) const
  {
    Token* t = new Token( Mod_Basic, TOK_LONG, TYP_OPERAND );
    t->lval = v;
    return t;
  }
  Token* operator()( double v ) const
  {
    Token* t = new Token( Mod_Basic, TOK_DOUBLE, TYP_OPERAND );
    t->dval = v;
    return t;
  }
  Token* operator()( bool v ) const
  {
    Token* t = new Token( Mod_Basic, TOK_BOOLEAN, TYP_OPERAND );
    t->lval = v ? 1 : 0;
    return t;
  }
  Token* operator()( const std::string& v ) const
  {
    Token* t = new Token( Mod_Basic, TOK_STRING, TYP_OPERAND );
    t->copyStr( v.c_str() );
    return t;
  }
};

// create variant from Token
token_variant getVariant( Token* t )
{
  token_variant v;
  switch ( t->id )
  {
  case TOK_LONG:
    v = t->lval;
    break;
  case TOK_DOUBLE:
    v = t->dval;
    break;
  case TOK_BOOLEAN:
    v = t->lval ? true : false;
    break;
  case TOK_STRING:
    v = std::string( t->tokval() );
    break;
  default:
    v = nullptr;
    break;
  }
  return v;
}
}  // namespace

Token* CompilerOptimization::optimize( Token* left, Token* oper, Token* right )
{
  static const auto null_variant = token_variant( nullptr );
  INFO_PRINT << "ANY TES\n";
  INFO_PRINT << ( *left ) << " " << ( *right ) << "\n";
  auto leftv = getVariant( left );
  if ( leftv == null_variant )
    return nullptr;
  auto rightv = getVariant( right );
  if ( rightv == null_variant )
    return nullptr;

  auto visitor = binaryop_token_visitor();
  visitor.id = oper->id;
  auto res = boost::apply_visitor( visitor, leftv, rightv );
  Token* ntoken = boost::apply_visitor( variant_to_token_visitor(), res );
  if ( ntoken )
    INFO_PRINT << "RESULT " << ( *ntoken ) << "\n";
  else
    INFO_PRINT << "No result\n";

  return ntoken;
}

Token* CompilerOptimization::optimize( Token* tok, Token* oper )
{
  INFO_PRINT << "UNARY TES\n";
  INFO_PRINT << ( *oper ) << " " << ( *tok ) << "\n";
  static const auto null_variant = token_variant( nullptr );
  auto tokv = getVariant( tok );
  if ( tokv == null_variant )
    return nullptr;

  auto visitor = unaryop_token_visitor();
  visitor.id = oper->id;
  auto res = boost::apply_visitor( visitor, tokv );
  Token* ntoken = boost::apply_visitor( variant_to_token_visitor(), res );
  if ( ntoken )
    INFO_PRINT << "RESULT " << ( *ntoken ) << "\n";
  else
    INFO_PRINT << "No result\n";

  return ntoken;
}
}  // namespace Bscript
}  // namespace Pol
