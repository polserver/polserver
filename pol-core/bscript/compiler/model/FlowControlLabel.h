#ifndef POLSERVER_FLOWCONTROLLABEL_H
#define POLSERVER_FLOWCONTROLLABEL_H

#include <vector>

#include <boost/optional.hpp>

namespace Pol::Bscript::Compiler
{
class InstructionEmitter;

class FlowControlLabel
{
public:
  FlowControlLabel();

  bool has_address() const;
  unsigned address() const;
  const std::vector<unsigned>& get_referencing_instruction_addresses() const;

  void assign_address( unsigned );
  void add_referencing_instruction_address( unsigned );

private:
  boost::optional<unsigned> maybe_address;
  std::vector<unsigned> referencing_instruction_addresses;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_FLOWCONTROLLABEL_H
