#ifndef POLSERVER_FLOWCONTROLLABEL_H
#define POLSERVER_FLOWCONTROLLABEL_H

#include <optional>
#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class FunctionParameterDeclaration;
class InstructionEmitter;
class UserFunction;

class FlowControlLabel
{
public:
  FlowControlLabel();

  bool has_address() const;
  unsigned address() const;
  const std::vector<unsigned>& get_referencing_instruction_addresses() const;

  void assign_address( unsigned );
  void add_referencing_instruction_address( unsigned );

  static std::string label_for_user_function_default_argument(
      const UserFunction&, const FunctionParameterDeclaration& );

private:
  std::optional<unsigned> maybe_address;
  std::vector<unsigned> referencing_instruction_addresses;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FLOWCONTROLLABEL_H
