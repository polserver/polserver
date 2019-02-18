#ifndef POL_MAIN_H
#define POL_MAIN_H

#include "../clib/Program/ProgramMain.h"

namespace Pol
{
namespace Clib
{
class PolMain final : public ProgramMain
{
public:
  PolMain();
  virtual ~PolMain();

protected:
  virtual int main();

private:
  virtual void showHelp();
};
}
}  // namespaces

#endif  // POL_MAIN_H
