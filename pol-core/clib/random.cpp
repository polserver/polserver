/** @file
 *
 * @par History
 * - 2008/07/08 Turley: Added Random Number Generator "lagged Fibonacci generator"
 * - 2014/08/01 Kimungu: usage of c++11 random lib (mersenne twister)
 */


#include <chrono>
#include <random>

#include "random.h"


namespace Pol::Clib
{
namespace
{
static std::mt19937 generator(
    static_cast<unsigned>( std::chrono::system_clock::now().time_since_epoch().count() ) );
}

// returns [0,f]
double random_double( double f )
{
  if ( f <= 0 )
    return 0;
  std::uniform_real_distribution<double> distribution( 0, f );
  return distribution( generator );
}

// returns [0,i]
int random_int( int i )
{
  if ( i == 0 )
    return i;
  std::uniform_int_distribution<int> distribution( 0, i );
  return distribution( generator );
}

// returns [minI,maxI]
int random_int_range( int minI, int maxI )
{
  if ( maxI < minI )
    std::swap( maxI, minI );
  if ( minI == maxI )
    return minI;
  std::uniform_int_distribution<int> distribution( minI, maxI );
  return distribution( generator );
}
}  // namespace Pol::Clib
