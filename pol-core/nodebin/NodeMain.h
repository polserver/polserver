#ifndef NODEBIN_MAIN_H
#define NODEBIN_MAIN_H

#include "../clib/Program/ProgramMain.h"

#include <string>

namespace Pol
{
namespace Clib
{
class NodeMain final : public ProgramMain
{
public:
  NodeMain();
  virtual ~NodeMain();
  virtual void showHelp();

protected:
  virtual int main();

private:
};
}  // namespace Clib
}  // namespace Pol

#endif  // RUN_ECL_MAIN_H
