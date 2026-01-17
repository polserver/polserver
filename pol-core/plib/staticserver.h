/** @file
 *
 * @par History
 * - 2005/06/01 Shinigami: added getstatics - to fill a list with statics
 */


#ifndef PLIB_STATICSERVER_H
#define PLIB_STATICSERVER_H

#include <vector>

#include "realmdescriptor.h"
#include "staticblock.h"


namespace Pol::Plib
{
class StaticEntryList;
}  // namespace Pol::Plib


namespace Pol::Plib
{
class StaticServer
{
public:
  explicit StaticServer( const RealmDescriptor& descriptor );
  ~StaticServer() = default;
  StaticServer& operator=( const StaticServer& ) { return *this; }
  bool findstatic( unsigned short x, unsigned short y, unsigned short objtype ) const;
  void getstatics( StaticEntryList& statics, unsigned short x, unsigned short y ) const;
  size_t sizeEstimate() const;

protected:
  void Validate() const;
  void ValidateBlock( unsigned short x, unsigned short y ) const;

private:
  const RealmDescriptor _descriptor;

  std::vector<STATIC_INDEX> _index;
  std::vector<STATIC_ENTRY> _statics;
};
}  // namespace Pol::Plib

#endif
