#ifndef POLSERVER_WARNON_H
#define POLSERVER_WARNON_H

namespace Pol::Bscript::Compiler
{
enum class WarnOn
{
  Never,
  IfNotUsed,
  IfUsed
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_WARNON_H
