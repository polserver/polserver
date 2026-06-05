#pragma once

#include "bobject.h"
#include "bobjectimp.h"

namespace Pol::Bscript
{
class BSpread final : public BObjectImp
{
public:
  BSpread( BObjectRef obj );
  BSpread( const BSpread& B );

private:
  ~BSpread() override = default;

public:
  size_t sizeEstimate() const override;

public:  // Class Machinery
  BObjectImp* copy() const override;
  bool isTrue() const override;
  std::string getStringRep() const override;

  BObjectRef object;
};
}  // namespace Pol::Bscript
