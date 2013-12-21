/*
History
=======
2005/06/01 Shinigami: Added Walking_Mounted and Running_Mounted movecosts

Notes
=======

*/

#ifndef MOVECOST_H
#define MOVECOST_H
namespace Pol {
  namespace Mobile {
    class Character;
  }
  namespace Core {
	unsigned short movecost( const Mobile::Character* chr, int carry_perc, bool running, bool mounted );
  }
}
#endif
