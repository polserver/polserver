#pragma once

#include "bscript/bobjectimp.h"
#include "clib/fixalloc.h"
#include "clib/passert.h"

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
  int increment() { return lval_ = wrap_add( lval_, 1 ); }

public:  // Class Machinery
  BObjectImp* copy() const override;
  BObjectImp* inverse() const override { return new BLong( wrap_neg( lval_ ) ); }
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

  BObjectImp* selfBitShiftLeftObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfBitShiftLeftObj( const BLong& objimp ) const override;

  BObjectImp* selfBitAndObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfBitAndObj( const BLong& objimp ) const override;

  BObjectImp* selfBitXorObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfBitXorObj( const BLong& objimp ) const override;

  BObjectImp* selfBitOrObjImp( const BObjectImp& objimp ) const override;
  BObjectImp* selfBitOrObj( const BLong& objimp ) const override;

  BObjectImp* bitnot() const override;

  void selfPlusPlus() override { lval_ = wrap_add( lval_, 1 ); }
  void selfMinusMinus() override { lval_ = wrap_sub( lval_, 1 ); }

  std::string getStringRep() const override;

  // eScript Integer arithmetic is defined here rather than left to C++, whose signed overflow,
  // INT_MIN/-1 and out-of-range shift counts are all undefined behavior. These reproduce what the
  // VM has always produced on x86 -- two's-complement wrap, shift counts masked to 0-31 -- so no
  // script changes meaning, and the compiler's constant folder gets the same answers by calling
  // the same operators (see specs/escript/03).
  static constexpr int wrap_add( int a, int b )
  {
    return static_cast<int>( static_cast<unsigned>( a ) + static_cast<unsigned>( b ) );
  }
  static constexpr int wrap_sub( int a, int b )
  {
    return static_cast<int>( static_cast<unsigned>( a ) - static_cast<unsigned>( b ) );
  }
  static constexpr int wrap_mul( int a, int b )
  {
    return static_cast<int>( static_cast<unsigned>( a ) * static_cast<unsigned>( b ) );
  }
  static constexpr int wrap_neg( int a )
  {
    return static_cast<int>( 0u - static_cast<unsigned>( a ) );
  }
  // Callers must reject a zero divisor first; that is an error object, not a value.
  static constexpr int wrap_div( int a, int b ) { return ( b == -1 ) ? wrap_neg( a ) : a / b; }
  static constexpr int wrap_mod( int a, int b ) { return ( b == -1 ) ? 0 : a % b; }
  static constexpr int shift_left( int v, int count )
  {
    unsigned shift = static_cast<unsigned>( count ) & 31u;
    return static_cast<int>( static_cast<unsigned>( v ) << shift );
  }
  static constexpr int shift_right( int v, int count )
  {
    return v >> ( static_cast<unsigned>( count ) & 31u );
  }

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
