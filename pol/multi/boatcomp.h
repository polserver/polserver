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

class UPlank : public ULockable
{
    typedef ULockable base;
public:
    void setboat( UBoat* boat );
protected:
    explicit UPlank( const ItemDesc& descriptor );
    friend Item* Item::create( const ItemDesc& descriptor, u32 serial );

    virtual void destroy();
    virtual class BObjectImp* get_script_member( const char* membername ) const;
    virtual class BObjectImp* get_script_member_id( const int id ) const; ///id test
    
private:
    ref_ptr<UBoat> boat_;
};

#endif
