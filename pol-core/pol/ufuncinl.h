/** @file
 *
 * @par History
 */


#ifndef UFUNCINL_H
#define UFUNCINL_H

#include <stdlib.h>
namespace Pol
{
namespace Core
{
inline bool inrangex_inline( const Mobile::Character* c1, const Mobile::Character* c2, int maxdist )
{
  return ( ( abs( c1->x() - c2->x() ) <= maxdist ) && ( abs( c1->y() - c2->y() ) <= maxdist ) );
}

inline bool inrangex_inline( const UObject* c1, unsigned short x, unsigned short y, int maxdist )
{
  return ( ( abs( c1->x() - x ) <= maxdist ) && ( abs( c1->y() - y ) <= maxdist ) );
}
}  // namespace Core
}  // namespace Pol
#endif
