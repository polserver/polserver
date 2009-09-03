/*
History
=======
2005/06/01 Shinigami: Added Walking_Mounted and Running_Mounted movecosts

Notes
=======

*/

#ifndef MOVECOST_H
#define MOVECOST_H

class Character;

unsigned short movecost( const Character* chr, int carry_perc, bool running, bool mounted );

#endif
