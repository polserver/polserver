/** @file
 *
 * @par History
 * - 2005/09/03 Shinigami: GetSuccessors - added support for non-blocking doors
 */


#ifndef __UOPATHNODE_H
#define __UOPATHNODE_H
// AStar search class
#include "../plib/stlastar.h"
#include "base/position.h"
#include "realms.h"
#include "realms/realm.h"

namespace Pol
{
namespace Core
{
#define BORDER_SKIRT 5
class AStarBlockers
{
public:
  int xLow, xHigh, yLow, yHigh;

  struct BlockNode
  {
    short x;
    short y;
    short z;
  };

  typedef std::vector<BlockNode*> BlockNodeVector;

public:
  AStarBlockers( short xL, short xH, short yL, short yH )
  {
    xLow = xL;
    xHigh = xH;
    yLow = yL;
    yHigh = yH;
  }

  void AddBlocker( short x, short y, short z )
  {
    BlockNode* theNode = new BlockNode;

    theNode->x = x;
    theNode->y = y;
    theNode->z = z;

    m_List.push_back( theNode );
  }

  ~AStarBlockers()
  {
    for ( auto blockNode : m_List )
    {
      delete blockNode;
    }
  }

  bool IsBlocking( short x, short y, short z )
  {
    for ( const auto blockNode : m_List )
    {
      if ( ( blockNode->x == x ) && ( blockNode->y == y ) &&
           ( abs( blockNode->z - z ) < settingsManager.ssopt.default_character_height ) )
        return true;
    }
    return false;
  }
  BlockNodeVector m_List;
};

class UOPathState
{
public:
  AStarBlockers* theBlockers;
  short x;
  short y;
  short z;
  Realms::Realm* realm;

  UOPathState()
      : theBlockers( nullptr ),
        x( 0 ),
        y( 0 ),
        z( 0 ),
        realm( Core::find_realm( std::string( "britannia" ) ) ){};
  UOPathState( short newx, short newy, short newz, Realms::Realm* newrealm,
               AStarBlockers* blockers )
  {
    x = newx;
    y = newy;
    z = newz;
    realm = newrealm;
    theBlockers = blockers;
  };
  float GoalDistanceEstimate( UOPathState& nodeGoal );
  bool IsGoal( UOPathState& nodeGoal );
  bool GetSuccessors( Plib::AStarSearch<UOPathState>* astarsearch, UOPathState* parent_node,
                      bool doors_block );
  float GetCost( UOPathState& successor );
  bool IsSameState( UOPathState& rhs );
  std::string Name();
};
bool UOPathState::IsSameState( UOPathState& rhs )
{
  return ( ( rhs.x == x ) && ( rhs.y == y ) && ( rhs.z == z ) && ( rhs.realm == realm ) );
}
float UOPathState::GoalDistanceEstimate( UOPathState& nodeGoal )
{
  return ( (float)( abs( x - nodeGoal.x ) + abs( y - nodeGoal.y ) + abs( z - nodeGoal.z ) ) );
}
bool UOPathState::IsGoal( UOPathState& nodeGoal )
{
  return ( ( nodeGoal.x == x ) && ( nodeGoal.y == y ) &&
           ( abs( nodeGoal.z - z ) <= settingsManager.ssopt.default_character_height ) );
  // return (IsSameState(nodeGoal));
}
float UOPathState::GetCost( UOPathState& successor )
{
  int xdiff = abs( x - successor.x );
  int ydiff = abs( y - successor.y );
  if ( xdiff && ydiff )
    return 1.414f;
  else
    return 1.0f;
}
std::string UOPathState::Name()
{
  fmt::Writer writer;
  writer.Format( "({},{},{})" ) << x << y << z;
  return writer.str();
}
bool UOPathState::GetSuccessors( Plib::AStarSearch<UOPathState>* astarsearch,
                                 UOPathState* /*parent_node*/, bool doors_block )
{
  Multi::UMulti* supporting_multi = nullptr;
  Items::Item* walkon_item = nullptr;

  UOPathState SolutionStartNode = ( *( astarsearch->GetSolutionStart() ) );
  UOPathState SolutionEndNode = ( *( astarsearch->GetSolutionEnd() ) );
  UOPathState* NewNode = new UOPathState();

  for ( short i = -1; i <= 1; i++ )
  {
    for ( short j = -1; j <= 1; j++ )
    {
      if ( ( i == 0 ) && ( j == 0 ) )
        continue;

      short newx = x + i;
      short newy = y + j;
      short newz = z;

      if ( ( newx < 0 ) || ( newx < ( theBlockers->xLow ) ) || ( newx > ( theBlockers->xHigh ) ) ||
           ( newx > ( (int)realm->width() ) ) )
        continue;

      if ( ( newy < 0 ) || ( newy < ( theBlockers->yLow ) ) || ( ( newy > theBlockers->yHigh ) ) ||
           ( newy > ( (int)realm->height() ) ) )
        continue;

      if ( realm->walkheight( Pos2d( newx, newy ), z, &newz, &supporting_multi, &walkon_item,
                              doors_block, Plib::MOVEMODE_LAND ) )
      {
        // Forbid diagonal move, if between 2 blockers - OWHorus {2011-04-26)
        bool blocked = false;
        if ( ( i != 0 ) && ( j != 0 ) )  // do only for diagonal moves
        {
          // If both neighbouring tiles are blocked, the move is illegal (diagonal move)
          if ( !realm->walkheight( Pos2d( x + i, y ), z, &newz, &supporting_multi, &walkon_item,
                                   doors_block, Plib::MOVEMODE_LAND ) )
            blocked = !( realm->walkheight( Pos2d( x, y + j ), z, &newz, &supporting_multi,
                                            &walkon_item, doors_block, Plib::MOVEMODE_LAND ) );
        }

        if ( !blocked )
        {
          NewNode->x = newx;
          NewNode->y = newy;
          NewNode->z = newz;
          NewNode->realm = realm;
          NewNode->theBlockers = theBlockers;

          if ( ( !NewNode->IsSameState( SolutionStartNode ) ) &&
               ( !NewNode->IsSameState( SolutionEndNode ) ) )
            blocked = ( theBlockers->IsBlocking( newx, newy, newz ) );
        }

        if ( !blocked )
        {
          if ( !astarsearch->AddSuccessor( *NewNode ) )
          {
            delete NewNode;
            return false;
          }
        }
      }
    }
  }

  delete NewNode;
  return true;
}
}  // namespace Core
}  // namespace Pol
#endif
