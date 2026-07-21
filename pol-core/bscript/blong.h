#pragma once

#include "../clib/fixalloc.h"
#include "../clib/passert.h"
#include "bobjectimp.h"

namespace Pol::Bscript
{

class BLong final : public BObjectImp
{
  using base = BObjectImp;

public:
#if BOBJECTIMP_DEBUG
  explicit BLong( int lval = 0L );
  BLong( const BLong& L );
#else
  explicit BLong( int lval = 0L ) : BObjectImp( OTLong ), lval_( static_cast<int>( lval ) ) {}
  BLong( const BLong& L ) : BObjectImp( OTLong ), lval_( L.lval_ ) {}
#endif
private:
  ~BLong() override = default;

public:
  void* operator new( std::size_t len );
  void operator delete( void* );
  void operator delete( void*, size_t );

  static BObjectImp* unpack( std::istream& is );
  static std::string pack( int val );
  void packonto( std::string& os ) const override;
  size_t sizeEstimate() const override;

  int value() const { return lval_; }
  int increment() { return ++lval_; }

public:  // Class Machinery
  BObjectImp* copy() const override;
  BObjectImp* inverse() const override { return new BLong( -lval_ ); }
  void copyvalue( const BLong& ni ) { lval_ = ni.lval_; }
  bool isTrue() const override;
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

  BObjectImp* selfModulusObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfModulusObj( const BLong& objimp ) const override;
  BObjectImp* selfModulusObj( const Double& objimp ) const override;
  void selfModulusObjImp( BObjectImp& objimp, BObject& obj ) override;
  void selfModulusObj( BLong& objimp, BObject& obj ) override;
  void selfModulusObj( Double& objimp, BObject& obj ) override;

  BObjectImp* selfBitShiftRightObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfBitShiftRightObj( const BLong& objimp ) const override;
  void selfBitShiftRightObjImp( BObjectImp& objimp, BObject& obj ) override;
  void selfBitShiftRightObj( BLong& objimp, BObject& obj ) override;

  BObjectImp* selfBitShiftLeftObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfBitShiftLeftObj( const BLong& objimp ) const override;
  void selfBitShiftLeftObjImp( BObjectImp& objimp, BObject& obj ) override;
  void selfBitShiftLeftObj( BLong& objimp, BObject& obj ) override;

  BObjectImp* selfBitAndObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfBitAndObj( const BLong& objimp ) const override;
  void selfBitAndObjImp( BObjectImp& objimp, BObject& obj ) override;
  void selfBitAndObj( BLong& objimp, BObject& obj ) override;

  BObjectImp* selfBitXorObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfBitXorObj( const BLong& objimp ) const override;
  void selfBitXorObjImp( BObjectImp& objimp, BObject& obj ) override;
  void selfBitXorObj( BLong& objimp, BObject& obj ) override;

  BObjectImp* selfBitOrObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfBitOrObj( const BLong& objimp ) const override;
  void selfBitOrObjImp( BObjectImp& objimp, BObject& obj ) override;
  void selfBitOrObj( BLong& objimp, BObject& obj ) override;

  BObjectImp* bitnot() const override;

  void selfPlusPlus() override { ++lval_; }
  void selfMinusMinus() override { --lval_; }

  std::string getStringRep() const override;

protected:
  int lval_;
};

extern Clib::fixed_allocator<BLong, 256> blong_alloc;
inline void* BLong::operator new( std::size_t len )
{
  (void)len;
  passert_paranoid( len == sizeof( BLong ) );
  return blong_alloc.allocate();
}
inline void BLong::operator delete( void* p )
{
  blong_alloc.deallocate( p );
}
inline void BLong::operator delete( void* p, size_t /*len*/ )
{
  blong_alloc.deallocate( p );
}
}  // namespace Pol::Bscript
