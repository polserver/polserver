/** @file
 *
 * @par History
 * - 2008/07/08 Turley: Added Random Number Generator "lagged Fibonacci generator"
 */


#ifndef RANDOM_H
#define RANDOM_H

namespace Pol
{
namespace Clib
{
double random_double( double f );

int random_int( int i );

int random_int_range( int minI, int maxI );
}
}
#endif //RANDOM_H
