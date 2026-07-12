#pragma once

#include "bobjectimp.h"

namespace Pol::Bscript
{
class BBoolean final : public BObjectImp
{
  using base = BObjectImp;

public:
#if BOBJECTIMP_DEBUG
  explicit BBoolean( bool bval = false );
  BBoolean( const BBoolean& B );
#else
  explicit BBoolean( bool bval = false ) : BObjectImp( OTBoolean ), bval_( bval ) {}
  BBoolean( const BBoolean& B ) : BBoolean( B.bval_ ) {}
#endif
private:
  ~BBoolean() override = default;

public:
  static BObjectImp* unpack( std::istream& is );
  void packonto( std::string& os ) const override;
  size_t sizeEstimate() const override;

  bool value() const { return bval_; }

public:  // Class Machinery
  BObjectImp* copy() const override;
  bool isTrue() const override;
  bool operator==( const BObjectImp& objimp ) const override;

  std::string getStringRep() const override;
  void printOn( std::ostream& ) const override;

private:
  bool bval_;
};
}  // namespace Pol::Bscript
