#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{

class ThisMemberAssignmentGatherer : public NodeVisitor
{
public:
  ThisMemberAssignmentGatherer();

  void visit_member_assignment( MemberAssignment& ) override;
  void visit_function_expression( FunctionExpression& ) override;

  std::vector<MemberAssignment*> assignments;
};

}  // namespace Pol::Bscript::Compiler
