#ifndef POLSERVER_VARIABLE_H
#define POLSERVER_VARIABLE_H

#include <string>

namespace Pol::Bscript::Compiler
{
class SourceLocation;

class Variable
{
public:
  Variable( bool global, std::string name, unsigned block_depth, size_t index, bool warn_if_used,
            bool warn_if_unused, const SourceLocation& source_location );

  void mark_used();
  bool was_used() const;

  const bool global;
  const std::string name;
  const unsigned block_depth;
  const size_t index;
  const bool warn_if_used;
  const bool warn_if_unused;
  const SourceLocation& source_location;

private:
  bool used;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SCRIPTVARIABLE_H
