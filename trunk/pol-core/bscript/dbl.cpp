/*
History
=======

Notes
=======

*/

#include "clib/stl_inc.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "clib/stlutil.h"

#include "berror.h"
#include "bobject.h"
#include "impstr.h"

string Double::pack() const
{
    OSTRINGSTREAM os;
    os << "r" << dval_;
    return OSTRINGSTREAM_STR(os);
}

void Double::packonto( ostream& os ) const
{
    os << "r" << dval_;
}

BObjectImp* Double::unpack( const char* pstr )
{
    double dv;
    ISTRINGSTREAM is(pstr);
    if (is >> dv)
    {
        return new Double( dv );
    }
    else
    {
        return new BError( "Error extracting Real value" );
    }
}

BObjectImp* Double::unpack( istream& is )
{
    double dv;
    if (is >> dv)
    {
        return new Double( dv );
    }
    else
    {
        return new BError( "Error extracting Real value" );
    }
}

unsigned long Double::sizeEstimate() const
{
    return sizeof(Double);
}


bool Double::isEqual(const BObjectImp& objimp) const
{
    if (objimp.isa( OTDouble ))
    {
        double diff = dval_ - ((Double&)objimp).dval_;
        return fabs(diff) < 0.00000001;
    }
    else if (objimp.isa( OTLong ))
    {
        double diff = dval_ - ((BLong&)objimp).value();
        return fabs(diff) < 0.00000001;
    }
    else
    {
        return false;
    }
}

bool Double::isLessThan(const BObjectImp& objimp) const
{
    if (objimp.isa( OTDouble ))
    {
        return (dval_ < ((Double&) objimp).dval_);
    }
    else if (objimp.isa( OTLong ))
    {
        return (dval_ < ((BLong&)objimp).value());
    }
    else
    {
        return false;
    }
}

bool Double::isGE( long val ) const
{
    return dval_ >= val;
}
bool Double::isGT( long val ) const
{
    return dval_ > val;
}

string Double::getStringRep() const
{
    OSTRINGSTREAM os;

    os << dval_;
    return OSTRINGSTREAM_STR(os);
}

BObjectImp* Double::selfPlusObjImp(const BObjectImp& objimp) const
{
  if (objimp.isa( OTDouble )) 
  {
	  return new Double( dval_ + ((Double&) objimp).dval_ );
  }
  else if (objimp.isa( OTLong ))
  {
	  return new Double( dval_ + ((BLong&) objimp).value() );
  }
  else if (objimp.isa( OTString )) 
  {
      return new String( getStringRep() + ((String&) objimp).data() );
  } 
  else 
  {
	  return copy();
  }
}

BObjectImp* Double::selfMinusObjImp(const BObjectImp& objimp) const
{
	if (objimp.isa( OTDouble )) 
	{
		return new Double( dval_ - ((Double&) objimp).dval_ );
	}
	if (objimp.isa( OTLong )) 
	{
		return new Double( dval_ - ((BLong&) objimp).value() );
	}
	else if (objimp.isa( OTString )) 
	{
		String str( getStringRep() );
		return str.selfMinusObjImp( objimp );
	} 
	else 
	{
		return copy();
	}
}

BObjectImp* Double::selfTimesObjImp(const BObjectImp& objimp) const
{
  if (objimp.isa( OTDouble )) 
  {
	  return new Double( dval_ * ((Double&) objimp).dval_);
  }
  else if (objimp.isa( OTLong ))
  {
      return new Double( dval_ * ((BLong&) objimp).value() );
  }
  else 
  {
	  return copy();
  }
}

BObjectImp* Double::selfDividedByObjImp(const BObjectImp& objimp) const
{
    if (objimp.isa( OTDouble )) 
    {
        return new Double( dval_ / ((Double&) objimp).dval_ );
    }
    else if (objimp.isa( OTLong ))
    {
        return new Double( dval_ / ((BLong&)objimp).value() );
    }
    else 
    {
        return copy();
    }
}

