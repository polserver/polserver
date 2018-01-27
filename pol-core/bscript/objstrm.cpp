/** @file
 *
 * @par History
 */


#include "bobject.h"
#include "impstr.h"

#include "../../lib/format/format.h"
#include <sstream>

namespace Pol
{
namespace Bscript
{
std::ostream& operator<<( std::ostream& os, const BObject& obj )
{
  obj.impptr()->printOn( os );
  return os;
}
fmt::Writer& operator<<( fmt::Writer& w, const BObject& obj )
{
  std::ostringstream os;
  obj.impptr()->printOn( os );
  w << os.str();
  return w;
}

std::ostream& operator<<( std::ostream& os, const BObjectImp& objimp )
{
  objimp.printOn( os );
  return os;
}
fmt::Writer& operator<<( fmt::Writer& w, const BObjectImp& objimp )
{
  std::ostringstream os;
  objimp.printOn( os );
  w << os.str();
  return w;
}

void BObject::printOn( std::ostream& os ) const
{
  os << "{ BObject: " << BObjectImp::typestr( objimp->type() ) << " }";
}

void BObjectImp::printOn( std::ostream& os ) const
{
  os << "{ BObjectImp: " << typestr( type() ) << " }";
}

void UninitObject::printOn( std::ostream& os ) const
{
  os << "{ Uninitialized Object }";
}

void ObjArray::printOn( std::ostream& os ) const
{
  os << " { Array[" << ref_arr.size() << "] }";
}

void String::printOn( std::ostream& os ) const
{
  os << '\"' << value_ << '\"';
}

#if 0
    void SubString::printOn(std::ostream& os ) const
	{
	  os << "{ \"";
	  os << ref_String->value_.substr( begin-1, len );

	  os << "\" }";
	}
#endif

void Double::printOn( std::ostream& os ) const
{
  // CHECKME: consider setting precision
  os << dval_;
}

void BLong::printOn( std::ostream& os ) const
{
  os << lval_;
}

void BBoolean::printOn( std::ostream& os ) const
{
  os << (bval_ ? "true" : "false");
}
}
}
