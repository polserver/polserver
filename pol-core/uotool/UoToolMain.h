#ifndef UO_TOOL_MAIN_H
#define UO_TOOL_MAIN_H

#include <string>

#include "../clib/Program/ProgramMain.h"

namespace Pol
{
namespace UoTool
{
class UoToolMain : public Pol::Clib::ProgramMain
{
public:
  UoToolMain();
  virtual ~UoToolMain();

protected:
  virtual int main();

private:
  virtual void showHelp();
  int uotool();
};
}
}  // namespaces

#endif  // UO_TOOL_MAIN_H
