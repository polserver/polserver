#ifndef POLSERVER_FLOWCONTROLSCOPE_H
#define POLSERVER_FLOWCONTROLSCOPE_H

#include <memory>
#include <string>

namespace Pol::Bscript::Compiler
{
class FlowControlLabel;
class FlowControlScopes;
class SourceLocation;

class FlowControlScope
{
public:
  FlowControlScope( FlowControlScopes&, const SourceLocation&, std::string name,
                    std::shared_ptr<FlowControlLabel> );
  ~FlowControlScope();
  FlowControlScope( const FlowControlScope& ) = delete;
  FlowControlScope& operator=( const FlowControlScope& ) = delete;

  const SourceLocation& source_location;
  const std::string name;
  const std::shared_ptr<FlowControlLabel> flow_control_label;
  const unsigned local_variables_size;
  const FlowControlScope* const parent_scope;

private:
  FlowControlScopes& flow_control_scopes;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FLOWCONTROLSCOPE_H
