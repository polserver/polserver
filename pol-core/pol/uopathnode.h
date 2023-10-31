#pragma once

/** @file
 *
 * @par History
 * - 2005/09/03 Shinigami: GetSuccessors - added support for non-blocking doors
 */

// AStar search class
#include "clib/clib.h"
#include "plib/stlastar.h"
#include "realms/realm.h"

#include "base/position.h"
#include "base/vector.h"

namespace Pol
{
namespace Core
{
class AStarParams
{
public:
  AStarParams( Range2d searchrange, bool doors_block, Plib::MOVEMODE movemode,
               Realms::Realm* realm )
      : m_range( std::move( searchrange ) ),
        m_blocker(),
        m_doors_block( doors_block ),
        m_movemode( movemode ),
        m_realm( realm )
  {
  }
  ~AStarParams() = default;

  void AddBlocker( Pos3d pos ) { m_blocker.push_back( std::move( pos ) ); }

  bool IsBlocking( const Pos3d& pos ) const
  {
    for ( const auto& blockNode : m_blocker )
    {
      if ( blockNode.xy() == pos.xy() &&
           ( abs( blockNode.z() - pos.z() ) < settingsManager.ssopt.default_character_height ) )
        return true;
    }
    return false;
  }
  bool inSearchRange( const Pos2d& pos ) const { return m_range.contains( pos ); };

  bool walkheight( const Pos2d& pos, s8 z, short* newz )
  {
    static Multi::UMulti* supporting_multi = nullptr;
    static Items::Item* walkon_item = nullptr;
    return m_realm->walkheight( pos, z, newz, &supporting_multi, &walkon_item, m_doors_block,
                                m_movemode );
  }

  Realms::Realm* realm() const { return m_realm; };

private:
  Range2d m_range;
  std::vector<Pos3d> m_blocker;
  bool m_doors_block;
  Plib::MOVEMODE m_movemode;
  Realms::Realm* m_realm;
};

class UOPathState
{
public:
  UOPathState();
  UOPathState( Pos3d p, AStarParams* astarparams );
  ~UOPathState() = default;

  float GoalDistanceEstimate( const UOPathState& nodeGoal ) const;
  bool IsGoal( const UOPathState& nodeGoal ) const;
  bool GetSuccessors( Plib::AStarSearch<UOPathState>* astarsearch, UOPathState* parent_node ) const;
  float GetCost( const UOPathState& successor ) const;
  bool IsSameState( const UOPathState& rhs ) const;
  std::string Name() const;

  const Pos3d& position() const;

private:
  AStarParams* params;
  Pos3d pos;
};

UOPathState::UOPathState() : params( nullptr ), pos(){};

UOPathState::UOPathState( Pos3d p, AStarParams* astarparams )
    : params( astarparams ), pos( std::move( p ) ){};

bool UOPathState::IsSameState( const UOPathState& rhs ) const
{
  return pos == rhs.pos;
}

float UOPathState::GoalDistanceEstimate( const UOPathState& nodeGoal ) const
{
  return (float)( abs( pos.x() - nodeGoal.pos.x() ) + abs( pos.y() - nodeGoal.pos.y() ) +
                  abs( pos.z() - nodeGoal.pos.z() ) );
}

bool UOPathState::IsGoal( const UOPathState& nodeGoal ) const
{
  return pos.xy() == nodeGoal.pos.xy() &&
         ( abs( nodeGoal.pos.z() - pos.z() ) <= settingsManager.ssopt.default_character_height );
}

float UOPathState::GetCost( const UOPathState& successor ) const
{
  int xdiff = abs( pos.x() - successor.pos.x() );
  int ydiff = abs( pos.y() - successor.pos.y() );
  if ( xdiff && ydiff )
    return 1.414f;
  return 1.0f;
}

std::string UOPathState::Name() const
{
  fmt::Writer writer;
  writer << pos;
  return writer.str();
}

bool UOPathState::GetSuccessors( Plib::AStarSearch<UOPathState>* astarsearch,
                                 UOPathState* /*parent_node*/ ) const
{
  auto* SolutionStartNode = astarsearch->GetSolutionStart();
  auto* SolutionEndNode = astarsearch->GetSolutionEnd();

  for ( const auto& newpos :
        Range2d( pos.xy() - Vec2d( 1, 1 ), pos.xy() + Vec2d( 1, 1 ), params->realm() ) )
  {
    if ( newpos == pos.xy() )
      continue;
    if ( !params->inSearchRange( newpos ) )
      continue;
    short newz;
    if ( !params->walkheight( newpos, pos.z(), &newz ) )
      continue;
    // Forbid diagonal move, if between 2 blockers - OWHorus {2011-04-26)
    if ( ( newpos.x() != pos.x() ) && ( newpos.y() != pos.y() ) )  // do only for diagonal moves
    {
      // If both neighbouring tiles are blocked, the move is illegal (diagonal move)
      if ( !params->walkheight( Pos2d( pos.xy() ).x( newpos.x() ), pos.z(), &newz ) &&
           !params->walkheight( Pos2d( pos.xy() ).y( newpos.y() ), pos.z(), &newz ) )
        continue;
    }

    UOPathState NewNode{ Pos3d( newpos, Clib::clamp_convert<s8>( newz ) ), params };

    if ( !NewNode.IsSameState( *SolutionStartNode ) && !NewNode.IsSameState( *SolutionEndNode ) &&
         params->IsBlocking( NewNode.pos ) )
      continue;

    if ( !astarsearch->AddSuccessor( NewNode ) )
      return false;
  }

  return true;
}

const Pos3d& UOPathState::position() const
{
  return pos;
}
}  // namespace Core
}  // namespace Pol
