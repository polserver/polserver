/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <stdlib.h>
#include <string.h>

#include "../clib/stlutil.h"

#include "berror.h"
#include "bobject.h"
#include "impstr.h"


#if BOBJECTIMP_DEBUG
BLong::BLong(int lval) : BObjectImp( OTLong ), lval_(static_cast<int>(lval)) 
{
}

BLong::BLong(const BLong& L) : BObjectImp( OTLong ), lval_(L.lval_) 
{
}
#endif



string BLong::pack() const
{
    OSTRINGSTREAM os;
    os << "i" << lval_;
    return OSTRINGSTREAM_STR(os);
}

string BLong::pack( int val )
{
	OSTRINGSTREAM os;
	os << "i" << val;
	return OSTRINGSTREAM_STR(os);
}


void BLong::packonto( ostream& os ) const
{
    os << "i" << lval_;
}

BObjectImp* BLong::unpack( const char* pstr )
{
    int lv;
    ISTRINGSTREAM is( pstr );
    if (is >> lv)
    {
        return new BLong(lv);
    }
    else
    {
        return new BError( "Error extracting Integer value" );
    }
}

BObjectImp* BLong::unpack( istream& is )
{
    int lv;
    if (is >> lv)
    {
        return new BLong(lv);
    }
    else
    {
        return new BError( "Error extracting Integer value" );
    }
}

BObjectImp *BLong::copy() const
{
    return new BLong( *this );
}

size_t BLong::sizeEstimate() const
{
    return sizeof(BLong);
}

bool BLong::isTrue() const
{
    return (lval_ != 0);
}

bool BLong::isEqual(const BObjectImp& objimp) const
{
    if (objimp.isa( OTLong ))
    {
        return lval_ == ((BLong&)objimp).lval_;
    }
    else if (objimp.isa( OTDouble ))
    {
        return lval_ == ((Double&)objimp).value();
    }
    else
    {
        return false;
    }
}

bool BLong::isLessThan(const BObjectImp& objimp) const
{
    if (objimp.isa( OTLong ))
    {
        return lval_ < ((BLong&)objimp).lval_;
    }
    else if (objimp.isa( OTDouble ))
    {
        return lval_ < ((Double&)objimp).value();
    }
    else
    {
        return false;
    }
}

bool BLong::isLE( const BObjectImp& objimp ) const
{
    return objimp.isGE( lval_ );
}
bool BLong::isLT( const BObjectImp& objimp ) const
{
    return objimp.isGT( lval_ );
}
bool BLong::isGT( int val ) const
{
    return (lval_ > val);
}
bool BLong::isGE( int val ) const
{
    return (lval_ >= val);
}

string BLong::getStringRep() const
{
    OSTRINGSTREAM os;

    os << lval_;

    return OSTRINGSTREAM_STR(os);
}

BObjectImp* BLong::selfPlusObjImp(const BObjectImp& objimp) const
{
    if (objimp.isa( OTLong ))
    {   
        return new BLong( lval_ + ((BLong&)objimp).lval_ );
    }
    else if (objimp.isa( OTDouble )) 
    {
        return new Double( lval_ +  ((Double&) objimp).value() );
    }
    else if (objimp.isa( OTString )) 
    {
        return new String( getStringRep() + ((String&)objimp).data() );
    } 
    else 
    {
        return copy();
    }
}

BObjectImp* BLong::selfMinusObjImp(const BObjectImp& objimp) const
{
	if (objimp.isa( OTLong ))
    {
        return new BLong( lval_ - ((BLong&) objimp).lval_ );
    }
    else if (objimp.isa( OTDouble )) 
	{
		return new Double( lval_ - ((Double&) objimp).value() );
	}
	else if (objimp.isa( OTString )) 
	{
        String s( getStringRep() );
        return s.selfMinusObjImp( objimp );
	} 
	else 
	{
		return copy();
	}
}

BObjectImp* BLong::selfTimesObjImp(const BObjectImp& objimp) const
{
    if (objimp.isa( OTLong ))
    {
        return new BLong( lval_ * ((BLong&)objimp).lval_ );
    }
    else if (objimp.isa( OTDouble )) 
    {
        return new Double( lval_ * ((Double&) objimp).value() );
    }
    else 
    {
        return copy();
    }
}

BObjectImp* BLong::selfDividedByObjImp(const BObjectImp& objimp) const
{
    if (objimp.isa( OTLong ))
    {
        int divisor = ((BLong&) objimp).lval_;
        if (!divisor)
            return new BError( "Divide by Zero" );
        else
            return new BLong( lval_ / divisor );
    }
    else if (objimp.isa( OTDouble )) 
    {
        double divisor = ((Double&) objimp).value();
        if (divisor == 0.0)
            return new BError( "Divide by Zero" );
        else
            return new Double( lval_ / divisor );
    }
    else 
    {
        return copy();
    }
}


BObjectImp* BLong::selfModulusObjImp(const BObjectImp& objimp) const
{
    if (objimp.isa( OTLong ))
    {
        int divisor = ((BLong&) objimp).lval_;
       
        if (!divisor)
            return new BError( "Divide by Zero" );
        else
            return new BLong( lval_ % divisor );
    }
    else if (objimp.isa( OTDouble )) 
    {
        int divisor = static_cast<int>(((Double&) objimp).value());
        if (divisor == 0)
            return new BError( "Divide by Zero" );
        else
            return new Double( lval_ % divisor );
    }
    else 
    {
        return copy();
    }
}

BObjectImp* BLong::selfBitShiftRightObjImp(const BObjectImp& objimp) const
{
    if (objimp.isa( OTLong ))
    {
        int rightlval = ((BLong&) objimp).lval_;
       
        return new BLong( lval_ >> rightlval );
    }
    else 
    {
        return copy();
    }
}

BObjectImp* BLong::selfBitShiftLeftObjImp(const BObjectImp& objimp) const
{
    if (objimp.isa( OTLong ))
    {
        int rightlval = ((BLong&) objimp).lval_;
       
        return new BLong( lval_ << rightlval );
    }
    else 
    {
        return copy();
    }
}

BObjectImp* BLong::selfBitAndObjImp(const BObjectImp& objimp) const
{
    if (objimp.isa( OTLong ))
    {
        int rightlval = ((BLong&) objimp).lval_;
       
        return new BLong( lval_ & rightlval );
    }
    else 
    {
        return copy();
    }
}
BObjectImp* BLong::selfBitOrObjImp(const BObjectImp& objimp) const
{
    if (objimp.isa( OTLong ))
    {
        int rightlval = ((BLong&) objimp).lval_;
       
        return new BLong( lval_ | rightlval );
    }
    else 
    {
        return copy();
    }
}
BObjectImp* BLong::selfBitXorObjImp(const BObjectImp& objimp) const
{
    if (objimp.isa( OTLong ))
    {
        int rightlval = ((BLong&) objimp).lval_;
       
        return new BLong( lval_ ^ rightlval );
    }
    else 
    {
        return copy();
    }
}
BObjectImp* BLong::bitnot() const
{
    return new BLong( ~lval_ );
}
