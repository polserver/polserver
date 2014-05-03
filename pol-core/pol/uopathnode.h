/*
History
=======
2005/09/03 Shinigami: GetSuccessors - added support for non-blocking doors

Notes
=======

*/

#ifndef __UOPATHNODE_H
#define __UOPATHNODE_H
// AStar search class
#include "../plib/stlastar.h"
#include "../plib/realm.h"

#include "realms.h"
#include "wrldsize.h"

namespace Pol {
#define BORDER_SKIRT 5
  class AStarBlockers
  {
  public:
    int xLow, xHigh, yLow, yHigh;

    class BlockNode
    {
    public:
      short x;
      short y;
      short z;
    };

    typedef vector< BlockNode * > BlockNodeVector;

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
      BlockNode * theNode;

      theNode = new BlockNode;

      theNode->x = x;
      theNode->y = y;
      theNode->z = z;

      m_List.push_back( theNode );
    }

    ~AStarBlockers()
    {
      for ( BlockNodeVector::iterator blockNode = m_List.begin(); blockNode != m_List.end(); ++blockNode )
        delete ( *blockNode );
    }

    bool IsBlocking( short x, short y, short z )
    {
      BlockNode * theNode;
      for ( BlockNodeVector::iterator blockNode = m_List.begin(), blockNode_end = m_List.end(); blockNode != blockNode_end; ++blockNode )
      {
        theNode = ( *blockNode );
        if ( ( theNode->x == x ) && ( theNode->y == y ) && ( abs( theNode->z - z ) < PLAYER_CHARACTER_HEIGHT ) )
          return true;
      }
      return false;
    }
    BlockNodeVector m_List;
  };

  class UOPathState
  {
  public:
    AStarBlockers * theBlockers;
    short x;
    short y;
    short z;
    Plib::Realm* realm;
    char myName[80];

    UOPathState()
    {
      x = 0;
      y = 0;
      z = 0;
      realm = Core::find_realm( string( "britannia" ) );
    };
    UOPathState( short newx, short newy, short newz, Plib::Realm* newrealm, AStarBlockers * blockers )
    {
      x = newx;
      y = newy;
      z = newz;
      realm = newrealm;
      theBlockers = blockers;
    };
    float GoalDistanceEstimate( UOPathState &nodeGoal );
    bool IsGoal( UOPathState &nodeGoal );
    bool GetSuccessors( AStarSearch<UOPathState> *astarsearch, UOPathState *parent_node, bool doors_block );
    float GetCost( UOPathState &successor );
    bool IsSameState( UOPathState &rhs );
    char * Name();
  };
  bool UOPathState::IsSameState( UOPathState &rhs )
  {
    //	return ((rhs.x == x) && (rhs.y == y) && (abs(rhs.z - z) <= PLAYER_CHARACTER_HEIGHT));
    return ( ( rhs.x == x ) && ( rhs.y == y ) && ( rhs.z == z ) && ( rhs.realm == realm ) );
  }
  float UOPathState::GoalDistanceEstimate( UOPathState &nodeGoal )
  {
    return ( (float)( abs( x - nodeGoal.x ) + abs( y - nodeGoal.y ) + abs( z - nodeGoal.z ) ) );
  }
  bool UOPathState::IsGoal( UOPathState &nodeGoal )
  {
    return ( ( nodeGoal.x == x ) && ( nodeGoal.y == y ) && ( abs( nodeGoal.z - z ) <= PLAYER_CHARACTER_HEIGHT ) );
    //	return (IsSameState(nodeGoal));
  }
  float UOPathState::GetCost( UOPathState &successor )
  {
    int xdiff = abs( x - successor.x );
    int ydiff = abs( y - successor.y );
    if ( xdiff && ydiff )
      return 1.414f;
    else
      return 1.0f;
  }
  char * UOPathState::Name()
  {
    sprintf( myName, "(%d,%d,%d)", x, y, z );
    return myName;
  }
  bool UOPathState::GetSuccessors( AStarSearch<UOPathState> *astarsearch, UOPathState *parent_node, bool doors_block )
  {
    UOPathState * NewNode;
    short i, j;
    short newx, newy, newz;
    Multi::UMulti* supporting_multi = NULL;
    Items::Item* walkon_item = NULL;
    bool blocked;
    UOPathState SolutionStartNode = ( *( astarsearch->GetSolutionStart() ) );
    UOPathState SolutionEndNode = ( *( astarsearch->GetSolutionEnd() ) );
    NewNode = new UOPathState();

    for ( i = -1; i <= 1; i++ )
    {
      for ( j = -1; j <= 1; j++ )
      {
        if ( ( i == 0 ) && ( j == 0 ) )
          continue;

        newx = x + i;
        newy = y + j;
        newz = z;

        if ( ( newx < 0 ) ||
             ( newx < ( theBlockers->xLow ) ) ||
             ( newx >( theBlockers->xHigh ) ) ||
             ( newx >( (int)realm->width() ) ) )
             continue;

        if ( ( newy < 0 ) ||
             ( newy < ( theBlockers->yLow ) ) ||
             ( ( newy > theBlockers->yHigh ) ) ||
             ( newy >( (int)realm->height() ) ) )
             continue;

        if ( realm->walkheight( newx, newy, z, &newz, &supporting_multi, &walkon_item, doors_block, Core::MOVEMODE_LAND ) )
        {
          // Forbid diagonal move, if between 2 blockers - OWHorus {2011-04-26)
          blocked = false;
          if ( ( i != 0 ) && ( j != 0 ) )	// do only for diagonal moves
          {
            // If both neighbouring tiles are blocked, the move is illegal (diagonal move)
            if ( !realm->walkheight( x + i, y, z, &newz, &supporting_multi, &walkon_item, doors_block, Core::MOVEMODE_LAND ) )
              blocked = !( realm->walkheight( x, y + j, z, &newz, &supporting_multi, &walkon_item, doors_block, Core::MOVEMODE_LAND ) );
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

}
#endif

