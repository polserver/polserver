/** @file
 *
 * @par History
 * - 2005/06/01 Shinigami: Added Walking_Mounted and Running_Mounted movecosts
 */


#ifndef MOVECOST_H
#define MOVECOST_H

namespace Pol
{
namespace Mobile
{
class Character;
}
namespace Core
{
#define MAX_CARRY_PERC 200

struct MovementCost
{
  double cost[MAX_CARRY_PERC + 1];
  double over;
};

unsigned short movecost( const Mobile::Character* chr, int carry_perc, bool running, bool mounted );
}
}
#endif
