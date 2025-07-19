#include "ShortCircuitWarning.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/BinaryOperatorShortCircuit.h"
#include "bscript/compiler/ast/ElementAssignment.h"
#include "bscript/compiler/ast/Function.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/MemberAssignment.h"
#include "bscript/compiler/ast/MemberAssignmentByOperator.h"
#include "bscript/compiler/ast/MethodCall.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/ast/UnaryOperator.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/VariableAssignmentStatement.h"
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "bscript/tokens.h"
#include "clib/strutil.h"

namespace Pol::Bscript::Compiler
{
ShortCircuitWarning::ShortCircuitWarning( Report& report ) : visitor( report ) {}

void ShortCircuitWarning::warn( CompilerWorkspace& workspace )
{
  workspace.top_level_statements->accept( *this );
  if ( auto& program = workspace.program )
  {
    program->accept( *this );
  }
  for ( auto& user_function : workspace.user_functions )
  {
    user_function->accept( *this );
  }
}
void ShortCircuitWarning::visit_binary_operator_short_circuit( BinaryOperatorShortCircuit& op )
{
  // check only right side, left is only needed if more compile
  // optimizations are done
  op.rhs().accept( visitor );
}

///////

ShortCircuitReporter::ShortCircuitReporter( Report& report ) : report( report ) {}

void ShortCircuitReporter::trigger( Node& op )
{
  report.warning( op.source_location,
                  "Short circuit change: \"{}\" will potentially not been executed.\n{}",
                  op.describe(), op.source_location.getSourceLine() );
}
void ShortCircuitReporter::visit_binary_operator( BinaryOperator& op )
{
  switch ( op.token_id )
  {
  case TOK_ASSIGN:
  case INS_ASSIGN_CONSUME:
  case TOK_PLUSEQUAL:
  case TOK_MINUSEQUAL:
  case TOK_TIMESEQUAL:
  case TOK_DIVIDEEQUAL:
  case TOK_MODULUSEQUAL:
  case TOK_ADDMEMBER:
  case TOK_DELMEMBER:
    trigger( op );
    return;

  default:
    visit_children( op );
    return;
  }
}
void ShortCircuitReporter::visit_element_assignment( ElementAssignment& op )
{
  trigger( op );
}
void ShortCircuitReporter::visit_function_call( FunctionCall& op )
{
  // Whitelist of module functions without sideeffects
  // not all, just some i found
  static std::map<std::string, std::vector<std::string>> modulefuncs = {
      { "attributes", { "getattribute" } },
      { "basicio", {} },
      { "basic",
        { "casc", "cascz", "typeofint", "cint", "cdbl", "cstr", "hex", "len", "lower", "upper",
          "sizeof", "typeof" } },
      { "cfgfile", { "getconfigint", "getconfigreal", "getconfigstring" } },
      { "math", { "abs" } },
      { "npc", { "self" } },
      { "util", {} },
      { "uo",
        { "accessible", "getobjproperty", "getglobalproperty", "getobjtypebyname",
          "checklineofsight", "distance", "coordinatedistance", "readgameclock", "checklosat",
          "getequipmentbylayer", "listmobilesatlocation", "listmobilesnearlocation",
          "listequippeditems", "getamount" } },

  };
  if ( auto mf = op.function_link->module_function_declaration() )
  {
    bool ignore = false;
    if ( auto mod = modulefuncs.find( Clib::strlowerASCII( mf->scope ) ); mod != modulefuncs.end() )
    {
      if ( mod->second.empty() )
        ignore = true;
      else if ( std::find( mod->second.begin(), mod->second.end(),
                           Clib::strlowerASCII( mf->name ) ) != mod->second.end() )
        ignore = true;
    }
    if ( ignore )
    {
      visit_children( op );
      return;
    }
  }
  trigger( op );
}
void ShortCircuitReporter::visit_member_assignment( MemberAssignment& op )
{
  trigger( op );
}

void ShortCircuitReporter::visit_member_assignment_by_operator( MemberAssignmentByOperator& op )
{
  // todo what
  trigger( op );
}
void ShortCircuitReporter::visit_variable_assignment_statement( VariableAssignmentStatement& op )
{
  // todo what
  trigger( op );
}


void ShortCircuitReporter::visit_method_call( MethodCall& op )
{
  trigger( op );
}
void ShortCircuitReporter::visit_unary_operator( UnaryOperator& op )
{
  switch ( op.token_id )
  {
  case TOK_UNPLUSPLUS:
  case TOK_UNMINUSMINUS:
  case TOK_UNPLUSPLUS_POST:
  case TOK_UNMINUSMINUS_POST:
  case INS_SET_MEMBER_ID_UNPLUSPLUS:
  case INS_SET_MEMBER_ID_UNMINUSMINUS:
  case INS_SET_MEMBER_ID_UNPLUSPLUS_POST:
  case INS_SET_MEMBER_ID_UNMINUSMINUS_POST:
    trigger( op );
    return;
  default:
    visit_children( op );
    return;
  }
}
}  // namespace Pol::Bscript::Compiler
