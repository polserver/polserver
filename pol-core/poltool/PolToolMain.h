#ifndef POL_TOOL_MAIN_H
#define POL_TOOL_MAIN_H

#include "../clib/Program/ProgramMain.h"


namespace Pol::Clib
{
class PolToolMain final : public ProgramMain
{
public:
  PolToolMain();
  ~PolToolMain() override = default;

protected:
  int main() override;

private:
  void showHelp() override;
  int mapdump();
  int unpackCompressedGump();
};
}  // namespace Pol::Clib


#endif  // POL_TOOL_MAIN_H
