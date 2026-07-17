
#pragma once

#include "../clib/fixalloc.h"
#include "../clib/passert.h"
#include "bobjectimp.h"

namespace Pol::Bscript
{
class Double final : public BObjectImp
{
  using base = BObjectImp;

public:
  explicit Double( double dval = 0.0 ) : BObjectImp( OTDouble ), dval_( dval ) {}
  Double( const Double& dbl ) : BObjectImp( OTDouble ), dval_( dbl.dval_ ) {}

  static std::string double_to_string( double val );

protected:
  ~Double() override = default;

public:
  void* operator new( std::size_t len );
  void operator delete( void* );

  static BObjectImp* unpack( std::istream& is );
  void packonto( std::string& str ) const override;
  size_t sizeEstimate() const override;

  double value() const { return dval_; }
  void copyvalue( const Double& dbl ) { dval_ = dbl.dval_; }
  double increment() { return ++dval_; }

public:  // Class Machinery
  bool isTrue() const override { return ( dval_ != 0.0 ); }
  BObjectImp* copy() const override { return new Double( *this ); }
  bool operator==( const BObjectImp& objimp ) const override;
  bool operator<( const BObjectImp& objimp ) const override;

  BObjectImp* selfPlusObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfPlusObj( const BLong& objimp ) const override;
  BObjectImp* selfPlusObj( const Double& objimp ) const override;
  BObjectImp* selfPlusObj( const String& objimp ) const override;
  void selfPlusObjImp( BObjectImp& objimp, BObject& obj ) override;
  void selfPlusObj( BLong& objimp, BObject& obj ) override;
  void selfPlusObj( Double& objimp, BObject& obj ) override;
  void selfPlusObj( String& objimp, BObject& obj ) override;

  BObjectImp* selfMinusObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfMinusObj( const BLong& objimp ) const override;
  BObjectImp* selfMinusObj( const Double& objimp ) const override;
  BObjectImp* selfMinusObj( const String& objimp ) const override;
  void selfMinusObjImp( BObjectImp& objimp, BObject& obj ) override;
  void selfMinusObj( BLong& objimp, BObject& obj ) override;
  void selfMinusObj( Double& objimp, BObject& obj ) override;
  void selfMinusObj( String& objimp, BObject& obj ) override;

  BObjectImp* selfTimesObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfTimesObj( const BLong& objimp ) const override;
  BObjectImp* selfTimesObj( const Double& objimp ) const override;
  void selfTimesObjImp( BObjectImp& objimp, BObject& obj ) override;
  void selfTimesObj( BLong& objimp, BObject& obj ) override;
  void selfTimesObj( Double& objimp, BObject& obj ) override;

  BObjectImp* selfDividedByObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfDividedByObj( const BLong& objimp ) const override;
  BObjectImp* selfDividedByObj( const Double& objimp ) const override;
  void selfDividedByObjImp( BObjectImp& objimp, BObject& obj ) override;
  void selfDividedByObj( BLong& objimp, BObject& obj ) override;
  void selfDividedByObj( Double& objimp, BObject& obj ) override;
  void selfPlusPlus() override { ++dval_; }
  void selfMinusMinus() override { --dval_; }

  BObjectImp* selfModulusObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfModulusObj( const BLong& objimp ) const override;
  BObjectImp* selfModulusObj( const Double& objimp ) const override;
  void selfModulusObjImp( BObjectImp& objimp, BObject& obj ) override;
  void selfModulusObj( BLong& objimp, BObject& obj ) override;
  void selfModulusObj( Double& objimp, BObject& obj ) override;

  BObjectImp* inverse() const override { return new Double( -dval_ ); }
  std::string getStringRep() const override;

private:
  double dval_;
};

extern Clib::fixed_allocator<Double, 256> double_alloc;
inline void* Double::operator new( std::size_t len )
{
  (void)len;
  passert_paranoid( len == sizeof( Double ) );
  return double_alloc.allocate();
}
inline void Double::operator delete( void* p )
{
  double_alloc.deallocate( p );
}
}  // namespace Pol::Bscript
