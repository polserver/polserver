#include "regexp.h"

namespace Pol::Bscript
{

BRegExp::BRegExp( const std::string& expr, boost::regex_constants::syntax_option_type flags,
                  bool global, bool multiline )
    : BObjectImp( OTRegExp ),
      regex_( expr, flags ),
      match_flags_( boost::regex_constants::match_default )
{
  if ( !multiline )
    match_flags_ |= boost::regex_constants::match_single_line;

  if ( !global )
    match_flags_ |= boost::regex_constants::format_first_only;
}

BRegExp::BRegExp( const BRegExp& i )
    : BObjectImp( OTRegExp ), regex_( i.regex_ ), match_flags_( i.match_flags_ )
{
}

BObjectImp* BRegExp::copy() const
{
  return new BRegExp( *this );
}

const char* BRegExp::typeOf() const
{
  return "RegExp";
}

u8 BRegExp::typeOfInt() const
{
  return OTRegExp;
}

std::string BRegExp::getStringRep() const
{
  return "<RegExp>";
}

size_t BRegExp::sizeEstimate() const
{
  return sizeof( BRegExp );
}

bool BRegExp::operator<( const BObjectImp& ) const
{
  return false;
}

bool BRegExp::isTrue() const
{
  return true;
}
}  // namespace Pol::Bscript
