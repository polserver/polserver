#ifndef UO_TOOL_MAIN_H
#define UO_TOOL_MAIN_H

#include <string>

#include "../clib/Program/ProgramMain.h"

namespace Pol
{
namespace UoTool
{
class UoToolMain final : public Pol::Clib::ProgramMain
{
public:
  UoToolMain();
  ~UoToolMain() override;

protected:
  int main() override;

private:
  void showHelp() override;
  int uotool();
};
}  // namespace UoTool
}  // namespace Pol

#endif  // UO_TOOL_MAIN_H
