/*
History
=======
2008/07/08 Turley: Added Random Number Generator "lagged Fibonacci generator"

Notes
=======

*/

#ifndef _RANDOM_H_
#define _RANDOM_H_

namespace Pol {
  namespace Clib {
    double random_double( double f );

    int random_int( int i );

    int random_int_range( int minI, int maxI );
  }
}
#endif
