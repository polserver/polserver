#ifndef POL_MAIN_H
#define POL_MAIN_H

#include "../clib/Program/ProgramMain.h"


namespace Pol::Clib
{
class PolMain final : public ProgramMain
{
public:
  PolMain();
  ~PolMain() override;

protected:
  int main() override;

private:
  void showHelp() override;
};
}  // namespace Pol::Clib


#endif  // POL_MAIN_H
