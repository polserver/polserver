#include "bspecialjump.h"

namespace Pol::Bscript
{
std::unique_ptr<BSpecialUserFuncJump> BSpecialUserFuncJump::imp_special_userjmp =
    std::make_unique<BSpecialUserFuncJump>();

BSpecialUserFuncJump::BSpecialUserFuncJump() : BObjectImp( OTSpecialUserFuncJump ) {}

size_t BSpecialUserFuncJump::sizeEstimate() const
{
  return sizeof( BSpecialUserFuncJump );
};

std::string BSpecialUserFuncJump::getStringRep() const
{
  return "BSpecialUserFuncJump";
};

BObjectImp* BSpecialUserFuncJump::copy() const
{
  return get();
};

BSpecialUserFuncJump* BSpecialUserFuncJump::get()
{
  return imp_special_userjmp.get();
}

void BSpecialUserFuncJump::ReleaseSharedInstance()
{
  imp_special_userjmp.reset();
}
}  // namespace Pol::Bscript
