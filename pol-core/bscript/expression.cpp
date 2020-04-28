/** @file
 *
 * @par History
 * - 2020/04/24 Syzygy:    Moved here from compiler.cpp
 */


#include "expression.h"

#include "../clib/passert.h"
#include "../clib/stlutil.h"
#include "compilercfg.h"
#include "objmembers.h"
#include "token.h"
#include "tokens.h"
#include "userfunc.h"

namespace Pol
{
namespace Bscript
{

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

void Expression::consume_tokens( Expression& expr )
{
  for( auto& token : expr.tokens ) {
    CA.push(token);
  }
  expr.tokens.clear();
}

Token* optimize_long_operation( Token* left, Token* oper, Token* right )
{
  int lval = 0;
  switch ( oper->id )
  {
  case TOK_ADD:
    lval = left->lval + right->lval;
    break;
  case TOK_SUBTRACT:
    lval = left->lval - right->lval;
    break;
  case TOK_MULT:
    lval = left->lval * right->lval;
    break;
  case TOK_DIV:
    if ( right->lval == 0 )
      throw std::runtime_error( "Program would divide by zero" );
    lval = left->lval / right->lval;
    break;

  case TOK_EQUAL:
    lval = ( left->lval == right->lval );
    break;
  case TOK_NEQ:
    lval = ( left->lval != right->lval );
    break;
  case TOK_LESSTHAN:
    lval = ( left->lval < right->lval );
    break;
  case TOK_LESSEQ:
    lval = ( left->lval <= right->lval );
    break;
  case TOK_GRTHAN:
    lval = ( left->lval > right->lval );
    break;
  case TOK_GREQ:
    lval = ( left->lval >= right->lval );
    break;

  case TOK_AND:
    lval = ( left->lval && right->lval );
    break;
  case TOK_OR:
    lval = ( left->lval || right->lval );
    break;

  case TOK_BSRIGHT:
    lval = ( left->lval >> right->lval );
    break;
  case TOK_BSLEFT:
    lval = ( left->lval << right->lval );
    break;
  case TOK_BITAND:
    lval = ( left->lval & right->lval );
    break;
  case TOK_BITOR:
    lval = ( left->lval | right->lval );
    break;
  case TOK_BITXOR:
    lval = ( left->lval ^ right->lval );
    break;

  default:
    return nullptr;

    break;
  }

  auto ntoken = new Token( *left );
  ntoken->lval = lval;
  return ntoken;
}

Token* optimize_double_operation( Token* left, Token* oper, Token* right )
{
  double dval = 0.0;

  switch ( oper->id )
  {
  case TOK_ADD:
    dval = left->dval + right->dval;
    break;
  case TOK_SUBTRACT:
    dval = left->dval - right->dval;
    break;
  case TOK_MULT:
    dval = left->dval * right->dval;
    break;
  case TOK_DIV:
    if ( right->dval == 0.0 )
      throw std::runtime_error( "Program would divide by zero" );
    dval = left->dval / right->dval;
    break;

  default:
    break;
  }

  auto ntoken = new Token( *left );
  ntoken->dval = dval;
  return ntoken;
}


Token* optimize_string_operation( Token* left, Token* oper, Token* right )
{
  Token* ntoken = nullptr;
  switch ( oper->id )
  {
  case TOK_ADD:
  {
    ntoken = new Token( *left );
    std::string combined;
    combined = std::string( left->tokval() ) + std::string( right->tokval() );
    ntoken->copyStr( combined.c_str() );
  }
    break;

  default:
    break;
  }
  return ntoken;
}

Token* optimize_long_operation( Token* oper, Token* value )
{
  Token* ntoken = nullptr;
  switch ( oper->id )
  {
  case TOK_UNMINUS:
    ntoken = new Token( *value );
    ntoken->lval = -value->lval;
    break;
  case TOK_LOG_NOT:
    ntoken = new Token( *value );
    ntoken->lval = !value->lval;
    break;
  case TOK_BITWISE_NOT:
    ntoken = new Token( *value );
    ntoken->lval = ~ntoken->lval;
    break;

  default:
    break;
  }
  return ntoken;
}
Token* optimize_double_operation( Token* oper, Token* value )
{
  Token* ntoken = nullptr;
  switch ( oper->id )
  {
  case TOK_UNMINUS:
    ntoken = new Token( *value );
    ntoken->dval = -value->dval;
    break;

  default:
    break;
  }
  return ntoken;
}
Token* optimize_string_operation( Token* /*oper*/, Token* /*value*/ )
{
  return nullptr;
}

void Expression::optimize_binary_operations()
{
  for ( unsigned i = 0; i < tokens.size(); i++ )
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
    if ( left->id != right->id )
    {
      // only optimize operations on like operands
      continue;
    }
    if ( left->id != TOK_LONG && left->id != TOK_STRING && left->id != TOK_DOUBLE )
    {
      continue;
    }

    Token* ntoken = nullptr;
    switch ( left->id )
    {
    case TOK_LONG:
      ntoken = optimize_long_operation( left, oper, right );
      break;
    case TOK_DOUBLE:
      ntoken = optimize_double_operation( left, oper, right );
      break;
    case TOK_STRING:
      ntoken = optimize_string_operation( left, oper, right );
      break;


    default:
      break;
    }
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
  for ( unsigned i = 0; i < tokens.size(); i++ )
  {
    Token* oper = tokens[i];
    if ( oper->type != TYP_UNARY_OPERATOR )
    {
      continue;
    }
    if ( i < 1 )
      throw std::runtime_error( "Unbalanced unary operator: " + Clib::tostring( *oper ) );

    Token* value = tokens[i - 1];

    if ( oper->id == TOK_UNPLUS )
    {
      // unary plus does nothing.
      delete oper;
      tokens.erase( tokens.begin() + i, tokens.begin() + i + 1 );
      continue;
    }
    if ( value->id != TOK_LONG && value->id != TOK_STRING && value->id != TOK_DOUBLE )
    {
      continue;
    }
    Token* ntoken = nullptr;
    switch ( value->id )
    {
    case TOK_LONG:
      ntoken = optimize_long_operation( oper, value );
      break;
    case TOK_DOUBLE:
      ntoken = optimize_double_operation( oper, value );
      break;
    case TOK_STRING:
      ntoken = optimize_string_operation( oper, value );
      break;
    default:
      break;
    }
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
      if ( objmemb != nullptr && compilercfg.OptimizeObjectMembers )
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
  for ( unsigned i = 1; i < tokens.size(); i++ )
  {
    if ( optimize_token( i ) )
      return;
  }
}

/*
    TODO:
    add options to:
    a) disable all optimization
    b) disable optimization on doubles
    OPTIMIZATIONS PERFORMED:
    Constant Long Arithmetic:  (+ - * /)
    5 + 7   ->   5 7 +   ->  12
    Constant Double Arithmetic: (+ - * /)
    5.5 + 6.5  ->  5.5 6.5 +   -> 12.0
    String Concatenation:    (+)
    "hello" + " world" -> "hello" " world" + -> "hello world"
    OPTIMIZATIONS TO BE CONSIDERED:
    More efficient constructs: (+=, -=, *=, /=)
    A := A + 5 -> A A 5 + := -> A 5 +=
    Note, this is harder for things like:
    A[5] := A[5] + 4;
    which requires reversing your way up the expression
    and searching for like operands.

    Optimizations that must take place at a different level:
    0  IFFALSE -> GOTO
    1  IFFALSE -> NOP
    0  IF    -> NOP
    1  IF    -> GOTO
    34: GOTO 56
    56: GOTO 78    --> 34: GOTO 78  56: GOTO 78
    NOP    -> (null) (compress NOPs)
    */
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


}  // namespace Bscript
}  // namespace Pol
