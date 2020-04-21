/** @file
 *
 * @par History
 */


#include "expression.h"

#include <string>

#include "../clib/logfacility.h"
#include "../clib/stlutil.h"
#include "modules.h"
#include "objmembers.h"
#include "token.h"
#include "tokens.h"
#include "userfunc.h"
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
    case TOK_MODULUS:
      return token_variant( v1 % v2 );
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
    case TOK_SUBTRACT:  // funny operation
    {
      auto v1s = tok_to_string( v1 );
      auto v2s = tok_to_string( v2 );
      auto pos = v1s.find( v2s );
      if ( pos != std::string::npos )
        v1s.erase( pos, v2s.size() );
      return token_variant( v1s );
    }
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

Token* Expression::optimize_operation( Token* left, Token* oper, Token* right ) const
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

Token* Expression::optimize_operation( Token* tok, Token* oper ) const
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

void Expression::optimize_binary_operations()
{
  for ( unsigned i = 0; i < tokens.size(); ++i )
  {
    Token* oper = tokens[i];
    if ( oper->type != TYP_OPERATOR )
    {
      continue;
    }
    if ( i < 2 )
      throw std::runtime_error( "Unbalanced binary operator: " + Clib::tostring( *oper ) );

    Token* left = tokens[i - 2];
    Token* right = tokens[i - 1];
    Token* ntoken = optimize_operation( left, oper, right );
    if ( ntoken )
    {
      delete left;
      delete right;
      delete oper;
      tokens[i - 2] = ntoken;
      tokens.erase( tokens.begin() + ( i - 1 ), tokens.begin() + ( i + 1 ) );
    }
  }
}

void Expression::optimize_unary_operations()
{
  for ( unsigned i = 0; i < tokens.size(); ++i )
  {
    Token* oper = tokens[i];
    if ( oper->type != TYP_UNARY_OPERATOR )
    {
      continue;
    }
    if ( i < 1 )
      throw std::runtime_error( "Unbalanced unary operator: " + Clib::tostring( *oper ) );

    Token* value = tokens[i - 1];

    Token* ntoken = optimize_operation( value, oper );
    if ( ntoken )
    {
      delete value;
      delete oper;
      tokens[i - 1] = ntoken;
      tokens.erase( tokens.begin() + i, tokens.begin() + ( i + 1 ) );
    }
  }
}

int Expression::get_num_tokens( int idx ) const
{
  Token* tkn = tokens[idx];
  int children = 0;

  if ( tkn->type == TYP_OPERAND )  // constant
  {
    // "anonymous" struct definitions inside array/dict/...
    if ( tkn->id == INS_ADDMEMBER_ASSIGN )  // struct{a:=1}
      children = 2;
    else if ( tkn->id == INS_ADDMEMBER2 )  // struct{a}
      children = 1;
    else
      children = 0;  // just myself
  }
  else if ( tkn->id == INS_ADDMEMBER_ASSIGN )
  {
    children = 2;
  }
  else if ( tkn->id == INS_DICTIONARY_ADDMEMBER )
  {
    children = 3;
  }
  else if ( tkn->id == INS_MULTISUBSCRIPT )
  {
    children = 1 + tkn->lval;
  }
  else if ( tkn->id == INS_MULTISUBSCRIPT_ASSIGN )
  {
    children = 1 + tkn->lval;
  }
  else if ( tkn->type == TYP_OPERATOR )  // binary operator
  {
    children = 2;
  }
  else if ( tkn->type == TYP_UNARY_OPERATOR )
  {
    children = 1;
  }
  else if ( tkn->type == TYP_FUNC )
  {
    children = static_cast<int>( tkn->userfunc->parameters.size() );
  }
  else if ( tkn->type == TYP_METHOD )
  {
    children = 1 + tkn->lval;
  }
  else if ( tkn->type == TYP_USERFUNC )
  {
    // the CTRL_JSR_USERFUNC
    // FIXME: TODO: what?
    children = 1;
  }
  else if ( tkn->id == CTRL_JSR_USERFUNC )
  {
    // the CTRL_MAKELOCAL + the parameters
    children = static_cast<int>( 1 + tkn->userfunc->parameters.size() );
  }
  else if ( tkn->id == CTRL_MAKELOCAL )
  {
    children = 0;
  }
  else
  {
    passert_always( 0 );
  }
  int count = 1;
  for ( int i = 0; i < children; ++i )
  {
    count += get_num_tokens( idx - count );
  }
  return count;
}

bool Expression::optimize_token( int i )
{
  Token* oper = tokens[i];
  if ( oper->type == TYP_OPERATOR && oper->id == TOK_ASSIGN )
  {
    int right_idx = i - 1;
    int left_idx = right_idx - get_num_tokens( i - 1 );
    if ( right_idx < 0 || left_idx < 0 )
    {
      throw std::runtime_error( "Unbalanced operator: " + Clib::tostring( *oper ) );
    }
    // Token* right = tokens[ right_idx ];
    Token* left = tokens[left_idx];
    if ( left->id == TOK_ARRAY_SUBSCRIPT )
    {
      oper->id = INS_SUBSCRIPT_ASSIGN;
      oper->type = TYP_UNARY_OPERATOR;
      oper->lval = left->lval;
      delete left;
      tokens.erase( tokens.begin() + left_idx, tokens.begin() + ( left_idx + 1 ) );
      return true;
    }
    else if ( left->id == INS_MULTISUBSCRIPT )
    {
      oper->id = INS_MULTISUBSCRIPT_ASSIGN;
      oper->type = TYP_UNARY_OPERATOR;
      oper->lval = left->lval;
      delete left;
      tokens.erase( tokens.begin() + left_idx, tokens.begin() + ( left_idx + 1 ) );
      return true;
    }
    else if ( left->id == INS_GET_MEMBER )
    {
      oper->id = INS_SET_MEMBER;
      oper->type = TYP_UNARY_OPERATOR;
      oper->copyStr( left->tokval() );
      delete left;
      tokens.erase( tokens.begin() + left_idx, tokens.begin() + ( left_idx + 1 ) );
      return true;
    }
    else if ( left->id == INS_GET_MEMBER_ID )
    {
      OSTRINGSTREAM os;
      os << left->lval;

      oper->id = INS_SET_MEMBER_ID;
      oper->type = TYP_UNARY_OPERATOR;
      oper->copyStr( OSTRINGSTREAM_STR( os ).c_str() );
      oper->lval = left->lval;
      delete left;
      tokens.erase( tokens.begin() + left_idx, tokens.begin() + ( left_idx + 1 ) );
      return true;
    }
  }
  else if ( oper->id == INS_ASSIGN_CONSUME )
  {
    int right_idx = i - 1;
    int left_idx = right_idx - get_num_tokens( i - 1 );
    // Token* right = tokens[ right_idx ];
    Token* left = tokens[left_idx];
    if ( left->id == TOK_LOCALVAR || left->id == TOK_GLOBALVAR )
    {
      // FIXME: assigning to a global/local, then consuming. we can do better, for this special
      // case.
      if ( left->id == TOK_LOCALVAR )
        oper->id = INS_ASSIGN_LOCALVAR;
      else if ( left->id == TOK_GLOBALVAR )
        oper->id = INS_ASSIGN_GLOBALVAR;
      oper->type = TYP_UNARY_OPERATOR;
      oper->lval = left->lval;
      oper->copyStr( left->tokval() );
      delete left;
      tokens.erase( tokens.begin() + left_idx, tokens.begin() + left_idx + 1 );
      return true;
    }
  }
  else if ( oper->id == TOK_CONSUMER )
  {
    Token* operand = tokens[i - 1];
    if ( operand->id == TOK_ASSIGN )
    {
      operand->id = INS_ASSIGN_CONSUME;
      delete oper;
      tokens.pop_back();
      return true;
    }
    else if ( operand->id == INS_SUBSCRIPT_ASSIGN )
    {
      operand->id = INS_SUBSCRIPT_ASSIGN_CONSUME;
      delete oper;
      tokens.pop_back();
      return true;
    }
    else if ( operand->id == INS_SET_MEMBER )
    {
      operand->id = INS_SET_MEMBER_CONSUME;
      delete oper;
      tokens.pop_back();
      return true;
    }
    else if ( operand->id == INS_SET_MEMBER_ID )
    {
      operand->id = INS_SET_MEMBER_ID_CONSUME;
      delete oper;
      tokens.pop_back();
      return true;
    }
  }
  else if ( oper->id == TOK_UNPLUSPLUS || oper->id == TOK_UNPLUSPLUS_POST ||
            oper->id == TOK_UNMINUSMINUS || oper->id == TOK_UNMINUSMINUS_POST )
  {
    if ( i > 0 )
    {
      Token* operand = tokens[i - 1];
      if ( operand->id == INS_GET_MEMBER_ID )
      {
        // TODO: spezial consume instruction? no need to copy value obto valuestack?
        if ( oper->id == TOK_UNPLUSPLUS )
          operand->id = INS_SET_MEMBER_ID_UNPLUSPLUS;
        else if ( oper->id == TOK_UNMINUSMINUS )
          operand->id = INS_SET_MEMBER_ID_UNMINUSMINUS;
        else if ( oper->id == TOK_UNPLUSPLUS_POST )
          operand->id = INS_SET_MEMBER_ID_UNPLUSPLUS_POST;
        else if ( oper->id == TOK_UNMINUSMINUS_POST )
          operand->id = INS_SET_MEMBER_ID_UNMINUSMINUS_POST;
        delete oper;
        tokens.erase( tokens.begin() + i );
      }
    }
    return true;
  }
  else if ( oper->id == TOK_MEMBER )
  {
    Token* operand = tokens[i - 1];
    if ( operand->id == TOK_STRING /*|| operand->id == INS_CALL_METHOD*/ )
    {
      ObjMember* objmemb = getKnownObjMember( operand->tokval() );
      if ( objmemb != nullptr )
      {
        // merge the member name with the member operator.
        oper->id = INS_GET_MEMBER_ID;
        oper->type = TYP_UNARY_OPERATOR;
        oper->lval = (int)objmemb->id;

        delete operand;
        tokens.erase( tokens.begin() + i - 1, tokens.begin() + i );
        // 1: local #0
        // 2: get member id 'warmode' (27)
        // 3: 1L
        // 4: +=
        // 5: #
        if ( i < 3 )  // has to be the first op
        {
          if ( tokens[tokens.size() - 1]->id == TOK_CONSUMER )
            operand = tokens[tokens.size() - 2];
          else
            operand = tokens[tokens.size() - 1];
          if ( operand->id == TOK_PLUSEQUAL || operand->id == TOK_MINUSEQUAL ||
               operand->id == TOK_TIMESEQUAL || operand->id == TOK_DIVIDEEQUAL ||
               operand->id == TOK_MODULUSEQUAL )
          {
            // 12: local #0
            // 13: 1L
            // 14: set member id 'warmode' (27) += #
            if ( tokens[tokens.size() - 1]->id == TOK_CONSUMER )
            {
              delete tokens.back();
              tokens.pop_back();  // delete consumer
            }

            if ( operand->id == TOK_PLUSEQUAL )
              oper->id = INS_SET_MEMBER_ID_CONSUME_PLUSEQUAL;
            else if ( operand->id == TOK_MINUSEQUAL )
              oper->id = INS_SET_MEMBER_ID_CONSUME_MINUSEQUAL;
            else if ( operand->id == TOK_TIMESEQUAL )
              oper->id = INS_SET_MEMBER_ID_CONSUME_TIMESEQUAL;
            else if ( operand->id == TOK_DIVIDEEQUAL )
              oper->id = INS_SET_MEMBER_ID_CONSUME_DIVIDEEQUAL;
            else if ( operand->id == TOK_MODULUSEQUAL )
              oper->id = INS_SET_MEMBER_ID_CONSUME_MODULUSEQUAL;
            delete operand;
            tokens.pop_back();                                           // delete +=
            tokens.erase( tokens.begin() + i - 1, tokens.begin() + i );  // remove setmember
            tokens.insert( tokens.end(), oper );                         // and append it
            OSTRINGSTREAM os;
            os << oper->lval;
            oper->copyStr( OSTRINGSTREAM_STR( os ).c_str() );
          }
        }
        return true;
      }
      else
      {
        // merge the member name with the member operator.
        oper->id = INS_GET_MEMBER;
        oper->type = TYP_UNARY_OPERATOR;
        oper->copyStr( operand->tokval() );

        delete operand;
        tokens.erase( tokens.begin() + i - 1, tokens.begin() + i );
        return true;
      }
    }
    else
    {
      throw std::runtime_error( "Expected an identifier to follow the member (.) operator." );
    }
  }

  return false;
}

void Expression::optimize_assignments()
{
  for ( unsigned i = 1; i < tokens.size(); ++i )
  {
    if ( optimize_token( i ) )
      return;
  }
}

void Expression::optimize()
{
  size_t starting_size;
  do
  {
    starting_size = tokens.size();
    optimize_binary_operations();
    optimize_unary_operations();
    optimize_assignments();

  } while ( tokens.size() != starting_size );
}

Expression::~Expression()
{
  while ( !tokens.empty() )
  {
    Token* tkn = tokens.back();
    tokens.pop_back();
    delete tkn;
  }


  while ( !TX.empty() )
  {
    Token* tkn = TX.top();
    TX.pop();
    delete tkn;
  }


  while ( !CA.empty() )
  {
    Token* tkn = CA.front();
    CA.pop();
    delete tkn;
  }
}

void Expression::eat( Expression& expr )
{
  while ( !expr.CA.empty() )
  {
    CA.push( expr.CA.front() );
    expr.CA.pop();
  }
}

void Expression::eat2( Expression& expr )
{
  while ( !expr.tokens.empty() )
  {
    CA.push( expr.tokens.front() );
    expr.tokens.erase( expr.tokens.begin() );
  }
}
}  // namespace Bscript
}  // namespace Pol
