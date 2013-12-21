/*
History
=======
2009/09/03 MuadDib:	  Changes for account related source file relocation
                      Changes for multi related source file relocation



Notes
=======

*/

#ifndef BOATCOMP_H
#define BOATCOMP_H

#include "boat.h"
#include "../lockable.h"
namespace Pol {
  namespace Bscript {
	class BObjectImp;
  }
  namespace Multi {
	class UPlank : public Core::ULockable
	{
	  typedef Core::ULockable base;
	public:
	  void setboat( UBoat* boat );
	protected:
	  explicit UPlank( const Items::ItemDesc& descriptor );
	  friend Items::Item* Items::Item::create( const Items::ItemDesc& descriptor, u32 serial );

	  virtual void destroy();
	  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const;
	  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const; ///id test

	private:
	  ref_ptr<UBoat> boat_;
	};
  }
}
#endif
