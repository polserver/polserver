/** @file
 *
 * @par History
 */


#ifndef CLIB_STRSET_H
#define CLIB_STRSET_H

#include <cstddef>
#include <set>
#include <string>

namespace Pol
{
namespace Clib
{
class StringSet
{
public:
  bool contains( const char* str ) const;
  void add( const char* str );
  void remove( const char* str );
  bool empty() const;

  void readfrom( const std::string& str );
  std::string extract() const;

  typedef std::set<std::string>::iterator iterator;
  iterator begin();
  iterator end();

  typedef std::set<std::string>::const_iterator const_iterator;
  const_iterator begin() const;
  const_iterator end() const;

  size_t estimatedSize() const;

private:
  std::set<std::string> strings_;
};
}
}
#endif  // CLIB_STRSET_H
