#ifndef POL_TOOL_MAIN_H
#define POL_TOOL_MAIN_H

#include "../clib/Program/ProgramMain.h"

namespace Pol
{
namespace Clib
{
class PolToolMain : public ProgramMain
{
public:
  PolToolMain();
  virtual ~PolToolMain();

protected:
  virtual int main();

private:
  virtual void showHelp();
  int mapdump();
  int unpackCompressedGump();
};
}
}  // namespaces

#endif  // POL_TOOL_MAIN_H
