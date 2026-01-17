/** @file
 *
 * @par History
 */

#include "strset.h"

#include "stlutil.h"


namespace Pol::Clib
{
bool StringSet::contains( const char* str ) const
{
  return ( strings_.find( str ) != strings_.end() );
}

void StringSet::add( const char* str )
{
  strings_.insert( str );
}
void StringSet::remove( const char* str )
{
  strings_.erase( str );
}

bool StringSet::empty() const
{
  return strings_.empty();
}

void StringSet::readfrom( const std::string& str )
{
  if ( !str.empty() )
  {
    std::string temp;
    ISTRINGSTREAM is( str );
    while ( is >> temp )
    {
      strings_.insert( temp );
    }
  }
}

std::string StringSet::extract() const
{
  std::string temp;
  for ( const auto& elem : strings_ )
  {
    temp += elem;
    temp += " ";
  }
  return temp;
}

StringSet::iterator StringSet::begin()
{
  return strings_.begin();
}

StringSet::iterator StringSet::end()
{
  return strings_.end();
}

StringSet::const_iterator StringSet::begin() const
{
  return strings_.begin();
}

StringSet::const_iterator StringSet::end() const
{
  return strings_.end();
}

size_t StringSet::estimatedSize() const
{
  return Clib::memsize( strings_ );
}
}  // namespace Pol::Clib
