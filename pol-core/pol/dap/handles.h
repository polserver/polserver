#ifndef DAP_HANDLES_H
#define DAP_HANDLES_H

#include <map>
#include <variant>

#include <dap/types.h>

namespace dap
{
struct Variable;
}  // namespace dap

namespace Pol
{
namespace Bscript
{
class BObjectRef;
class BObjectImp;
}  // namespace Bscript

namespace Network
{
namespace DAP
{
struct GlobalReference
{
};
using FrameReference = size_t;
using VariableReference = Pol::Bscript::BObjectRef;

using Reference = std::variant<GlobalReference, FrameReference, VariableReference>;

class Handles
{
public:
  static constexpr int START_HANDLE = 1000;
  Handles();

  void reset();

  int create( const Reference& value );

  Reference* get( int handle );

  /**
   * Takes a `BObjectRef` and sets the `type` and `value` members of the `dap::Variable. Sets the
   * `variableReference` member for structured variables (eg. structs and arrays).
   */
  void add_variable_details( const Bscript::BObjectRef& objref, dap::Variable& variable );

  dap::array<dap::Variable> to_variables( const Bscript::BObjectRef& objref );

private:
  int _nextHandle;
  std::map<int, Reference> _handleMap;
};
}  // namespace DAP
}  // namespace Network
}  // namespace Pol
#endif