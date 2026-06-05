#pragma once

#include "bobjectimp.h"

namespace Pol::Bscript
{

// very very special Imp, should never be actually used
// we need to tell the executor that in certain cases
// call_method and call_method_id do not return a BObjectImp
// and the ValueStack should not be modified
// should not be handled like a "imp" and never be passed to
// a BObject since its a singleton
class BSpecialUserFuncJump final : public BObjectImp
{
public:
  static BSpecialUserFuncJump* get();
  static void ReleaseSharedInstance();

public:  // needed minimal imp
  size_t sizeEstimate() const override;
  BObjectImp* copy() const override;
  std::string getStringRep() const override;

private:
  BSpecialUserFuncJump();
  ~BSpecialUserFuncJump() override = default;
  static std::unique_ptr<BSpecialUserFuncJump> imp_special_userjmp;
  friend std::unique_ptr<BSpecialUserFuncJump> std::make_unique<BSpecialUserFuncJump>();
  friend std::unique_ptr<BSpecialUserFuncJump>::deleter_type;
};
}  // namespace Pol::Bscript
