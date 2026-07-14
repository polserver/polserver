#pragma once

#include "../clib/fixalloc.h"
#include "../clib/refptr.h"
#include "bobjectimp.h"

#include <string>

namespace Pol::Bscript
{
class UninitObject final : public BObjectImp
{
public:
  UninitObject();
  UninitObject( const UninitObject& i );

  static UninitObject* SharedInstance;
  static ref_ptr<BObjectImp> SharedInstanceOwner;
  static BObjectRef SharedInstanceRef;

  BObjectImp* copy() const override;
  size_t sizeEstimate() const override;
  std::string getStringRep() const override { return "<uninitialized object>"; }
  void printOn( std::ostream& os ) const override;

  bool isTrue() const override;
  bool operator==( const BObjectImp& objimp ) const override;
  bool operator<( const BObjectImp& objimp ) const override;

  void* operator new( std::size_t len );
  void operator delete( void* );

  static UninitObject* create() { return SharedInstance; }
  static void ReleaseSharedInstance()
  {
    SharedInstanceOwner.clear();
    SharedInstance = nullptr;
  }
};
extern Clib::fixed_allocator<UninitObject, 256> uninit_alloc;

inline void* UninitObject::operator new( std::size_t /*len*/ )
{
  return uninit_alloc.allocate();
}

inline void UninitObject::operator delete( void* p )
{
  uninit_alloc.deallocate( p );
}
}  // namespace Pol::Bscript
