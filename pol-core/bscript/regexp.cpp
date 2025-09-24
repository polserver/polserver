#include "regexp.h"
#include <stdexcept>

namespace Pol::Bscript
{
BRegExp::BRegExp( const std::string& pattern, const std::string& flags )
    : BObjectImp( OTRegExp ),
      match_flags_( boost::regex_constants::match_single_line |
                    boost::regex_constants::format_first_only )
{
  boost::regex_constants::syntax_option_type flag = boost::regex_constants::ECMAScript;

  for ( const auto& ch : flags )
  {
    switch ( ch )
    {
    case 'i':
      flag |= boost::regex_constants::icase;
      break;
    case 'm':
      match_flags_ &= ~boost::regex_constants::match_single_line;
      break;
    case 'g':
      match_flags_ &= ~boost::regex_constants::format_first_only;
      break;
    default:
      throw std::runtime_error( "Invalid flag character" );
    }
  }

  regex_ = boost::regex( pattern, flag );
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
