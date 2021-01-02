#ifndef POL_TOOL_MAIN_H
#define POL_TOOL_MAIN_H

#include "../clib/Program/ProgramMain.h"

namespace Pol
{
namespace Clib
{
class PolToolMain final : public ProgramMain                                                            
{
public:
  PolToolMain();
  virtual ~PolToolMain() = default;

protected:
  virtual int main();

private:
  virtual void showHelp();
  int mapdump();
  int unpackCompressedGump();
};
}  // namespace Clib
}  // namespace Pol

#endif  // POL_TOOL_MAIN_H
