/** @file
 *
 * @par History
 * - 2005/09/03 Shinigami: SearchStep - added support for non-blocking doors
 */


// STL A* Search implementation
// Copyright 2001 Justin Heyes-Jones

// used for text debugging

#ifndef STLASTAR_H
#define STLASTAR_H

#include <iostream>
#include <stdio.h>
// #include <conio.h>
#include <assert.h>

// stl includes
#include <algorithm>
#include <set>
#include <vector>

// fast fixed size memory allocator, used for fast node memory management
#include "fsa.h"

// Fixed size memory allocator can be disabled to compare performance
// Uses std new and delete instead if you turn it off
#define USE_FSA_MEMORY 1


namespace Pol::Plib
{
// The AStar search class. UserState is the users state space type
template <class UserState>
class AStarSearch
{
public:  // data
  enum
  {
    SEARCH_STATE_NOT_INITIALISED,
    SEARCH_STATE_SEARCHING,
    SEARCH_STATE_SUCCEEDED,
    SEARCH_STATE_FAILED,
    SEARCH_STATE_OUT_OF_MEMORY,
    SEARCH_STATE_INVALID,
    SEARCH_STATE_SOLUTION_CORRUPTED
  };

  // A node represents a possible state in the search
  // The user provided state type is included inside this type

public:
  class Node
  {
  public:
    Node* parent;  // used during the search to record the parent of successor nodes
    Node* child;   // used after the search for the application to view the search in reverse

    float g;  // cost of this node + it's predecessors
    float h;  // heuristic estimate of distance to goal
    float f;  // sum of cumulative cost of predecessors and self and heuristic

    Node() : parent( 0 ), child( 0 ), g( 0.0f ), h( 0.0f ), f( 0.0f ) {}
    UserState m_UserState;
  };


  using NodeVector = std::vector<Node*>;
  using NodeVectorIterator = typename NodeVector::iterator;
  // For sorting the heap the STL needs compare function that lets us compare
  // the f value of two nodes
  class HeapCompare_f
  {
  public:
    bool operator()( const Node* x, const Node* y ) const { return x->f > y->f; }
  };


public:  // methods
  // constructor just initialises private data
  AStarSearch( int MaxNodes = 1000 )
      : m_State( SEARCH_STATE_NOT_INITIALISED ),
        m_Steps( 0 ),
        m_Start( nullptr ),
        m_Goal( nullptr ),
        m_CurrentSolutionNode( nullptr ),
        m_FixedSizeAllocator( MaxNodes ),
        m_AllocateNodeCount( 0 ),
        m_FreeNodeCount( 0 ),
        m_CancelRequest( false )
  {
  }

  // call at any time to cancel the search and free up all the memory
  void CancelSearch() { m_CancelRequest = true; }
  // Set Start and goal states
  void SetStartAndGoalStates( UserState& Start, UserState& Goal )
  {
    m_CancelRequest = false;

    m_Start = AllocateNode();
    m_Goal = AllocateNode();

    m_Start->m_UserState = Start;
    m_Goal->m_UserState = Goal;

    m_State = SEARCH_STATE_SEARCHING;

    // Initialise the AStar specific parts of the Start Node
    // The user only needs fill out the state information

    m_Start->g = 0;
    m_Start->h = m_Start->m_UserState.GoalDistanceEstimate( m_Goal->m_UserState );
    m_Start->f = m_Start->g + m_Start->h;
    m_Start->parent = 0;

    // Push the start node on the Open list

    m_OpenList.push_back( m_Start );  // heap now unsorted

    // Sort back element into heap
    push_heap( m_OpenList.begin(), m_OpenList.end(), HeapCompare_f() );

    // Initialise counter for search steps
    m_Steps = 0;
  }

  bool InOpenList( UserState& theState )
  {
    NodeVectorIterator openlist_result, openlist_end;
    for ( openlist_result = m_OpenList.begin(), openlist_end = m_OpenList.end();
          openlist_result != openlist_end; ++openlist_result )
    {
      if ( ( *openlist_result )->m_UserState.IsSameState( theState ) )
      {
        return true;
      }
    }
    return false;
  }

  bool InClosedList( UserState& theState )
  {
    if ( theState.IsGoal( m_Goal->m_UserState ) )
      return true;

    NodeVectorIterator closedlist_result, closedlist_end;
    for ( closedlist_result = m_ClosedList.begin(), closedlist_end = m_ClosedList.end();
          closedlist_result != closedlist_end; ++closedlist_result )
    {
      if ( ( *closedlist_result )->m_UserState.IsSameState( theState ) )
      {
        return true;
      }
    }
    return false;
  }

  bool AddToSolutionList( Node* theNode )
  {
    NodeVectorIterator solution_result, solution_end;
    for ( solution_result = m_SolutionList.begin(), solution_end = m_SolutionList.end();
          solution_result != solution_end; ++solution_result )
    {
      if ( ( *solution_result ) == theNode )
        return false;
    }

    m_SolutionList.push_back( theNode );
    return true;
  }

  // Advances search one step
  unsigned int SearchStep()
  {
    // Firstly break if the user has not initialised the search
    assert( ( m_State > SEARCH_STATE_NOT_INITIALISED ) && ( m_State < SEARCH_STATE_INVALID ) );
    // Next I want it to be safe to do a searchstep once the search has succeeded...
    if ( ( m_State == SEARCH_STATE_SUCCEEDED ) || ( m_State == SEARCH_STATE_FAILED ) )
    {
      return m_State;
    }

    // Failure is defined as emptying the open list as there is nothing left to
    // search...
    // New: Allow user abort
    if ( m_OpenList.empty() || m_CancelRequest )
    {
      FreeAllNodes();
      m_State = SEARCH_STATE_FAILED;
      return m_State;
    }

    // Incremement step count
    m_Steps++;

    // Pop the best node (the one with the lowest f)
    Node* n = m_OpenList.front();  // get pointer to the node
    pop_heap( m_OpenList.begin(), m_OpenList.end(), HeapCompare_f() );
    m_OpenList.pop_back();

    // Check for the goal, once we pop that we're done
    if ( n->m_UserState.IsGoal( m_Goal->m_UserState ) )
    {
      // The user is going to use the Goal Node he passed in
      // so copy the parent pointer of n
      m_Goal->parent = n->parent;

      // A special case is that the goal was passed in as the start state
      // so handle that here
      if ( n != m_Start )
      {
        // delete n;
        FreeNode( n );

        // set the child pointers in each node (except Goal which has no child)
        Node* nodeChild = m_Goal;
        Node* nodeParent = m_Goal->parent;

        do
        {
          if ( !InClosedList( nodeChild->m_UserState ) || !AddToSolutionList( nodeChild ) ||
               !nodeParent )
          {
            FreeAllNodes();

            m_State = SEARCH_STATE_SOLUTION_CORRUPTED;
            return m_State;
          }

          nodeParent->child = nodeChild;

          nodeChild = nodeParent;
          nodeParent = nodeParent->parent;
        } while ( nodeChild != m_Start );  // Start is always the first node by definition
      }


      // delete nodes that aren't needed for the solution
      FreeUnusedNodes();

      m_State = SEARCH_STATE_SUCCEEDED;

      return m_State;
    }
    else  // not goal
    {
      // We now need to generate the successors of this node
      // The user helps us to do this, and we keep the new nodes in
      // m_Successors ...

      m_Successors.clear();  // empty vector of successor nodes to n

      // User provides this functions and uses AddSuccessor to add each successor of
      // node 'n' to m_Successors
      bool ret =
          n->m_UserState.GetSuccessors( this, n->parent ? &n->parent->m_UserState : nullptr );

      if ( !ret )
      {
        // free the nodes that may previously have been added
        for ( NodeVectorIterator successor = m_Successors.begin(),
                                 successor_end = m_Successors.end();
              successor != successor_end; ++successor )
        {
          FreeNode( ( *successor ) );
        }

        m_Successors.clear();  // empty vector of successor nodes to n

        // free up everything else we allocated
        FreeAllNodes();

        m_State = SEARCH_STATE_OUT_OF_MEMORY;
        return m_State;
      }

      // Now handle each successor to the current node ...
      for ( NodeVectorIterator successor = m_Successors.begin(), successor_end = m_Successors.end();
            successor != successor_end; ++successor )
      {
        // The g value for this successor ...
        float newg = n->g + n->m_UserState.GetCost( ( *successor )->m_UserState );

        // Now we need to find whether the node is on the open or closed lists
        // If it is but the node that is already on them is better (lower g)
        // then we can forget about this successor

        // First linear search of open list to find node

        NodeVectorIterator openlist_result, openlist_end;

        for ( openlist_result = m_OpenList.begin(), openlist_end = m_OpenList.end();
              openlist_result != openlist_end; ++openlist_result )
        {
          if ( ( *openlist_result )->m_UserState.IsSameState( ( *successor )->m_UserState ) )
          {
            break;
          }
        }

        if ( openlist_result != openlist_end )
        {
          // we found this state on open

          if ( ( *openlist_result )->g <= newg )
          {
            FreeNode( ( *successor ) );

            // the one on Open is cheaper than this one
            continue;
          }
        }

        NodeVectorIterator closedlist_result, closedlist_end;

        for ( closedlist_result = m_ClosedList.begin(), closedlist_end = m_ClosedList.end();
              closedlist_result != closedlist_end; ++closedlist_result )
        {
          if ( ( *closedlist_result )->m_UserState.IsSameState( ( *successor )->m_UserState ) )
          {
            break;
          }
        }

        if ( closedlist_result != closedlist_end )
        {
          // we found this state on closed

          if ( ( *closedlist_result )->g <= newg )
          {
            // the one on Closed is cheaper than this one
            FreeNode( ( *successor ) );
            continue;
          }
        }

        // This node is the best node so far with this particular state
        // so lets keep it and set up its AStar specific data ...

        ( *successor )->parent = n;
        ( *successor )->g = newg;
        ( *successor )->h = ( *successor )->m_UserState.GoalDistanceEstimate( m_Goal->m_UserState );
        ( *successor )->f = ( *successor )->g + ( *successor )->h;

        // Remove successor from closed if it was on it

        if ( closedlist_result != closedlist_end )
        {
          // remove it from Closed
          FreeNode( ( *closedlist_result ) );
          m_ClosedList.erase( closedlist_result );
        }

        // Update old version of this node
        if ( openlist_result != openlist_end )
        {
          FreeNode( ( *openlist_result ) );
          m_OpenList.erase( openlist_result );

          // re-make the heap
          make_heap( m_OpenList.begin(), m_OpenList.end(), HeapCompare_f() );

          // make_heap rather than sort_heap is an essential bug fix
          // thanks to Mike Ryynanen for pointing this out and then explaining
          // it in detail. sort_heap called on an invalid heap does not work

          //               sort_heap( m_OpenList.begin(), m_OpenList.end(), HeapCompare_f() );

          //               assert( is_heap( m_OpenList.begin(), m_OpenList.end(),
          // HeapCompare_f() ) );
        }

        // heap now unsorted
        m_OpenList.push_back( ( *successor ) );

        // sort back element into heap
        push_heap( m_OpenList.begin(), m_OpenList.end(), HeapCompare_f() );
      }

      // push n onto Closed, as we have expanded it now

      m_ClosedList.push_back( n );

    }  // end else (not goal so expand)

    return m_State;  // Succeeded bool is false at this point.
  }

  // User calls this to add a successor to a list of successors
  // when expanding the search frontier
  bool AddSuccessor( UserState& State )
  {
    Node* node = AllocateNode();

    if ( node )
    {
      node->m_UserState = State;
      m_Successors.push_back( node );
      return true;
    }
    return false;
  }

  // Free the solution nodes
  // This is done to clean up all used Node memory when you are done with the
  // search
  void FreeSolutionNodes()
  {
    Node* n = m_Start;

    if ( m_Start->child )
    {
      do
      {
        Node* del = n;
        n = n->child;
        FreeNode( del );

        del = nullptr;

      } while ( n != m_Goal );
      FreeNode( n );  // Delete the goal
    }
    else
    {
      // if the start node is the solution we need to just delete the start and goal
      // nodes
      FreeNode( m_Start );
      FreeNode( m_Goal );
    }
  }

  // Functions for traversing the solution
  // Get start node
  UserState* GetSolutionStart()
  {
    m_CurrentSolutionNode = m_Start;
    if ( m_Start )
    {
      return &m_Start->m_UserState;
    }

    return nullptr;
  }

  // Get next node
  UserState* GetSolutionNext()
  {
    if ( m_CurrentSolutionNode )
    {
      if ( m_CurrentSolutionNode->child )
      {
        Node* child = m_CurrentSolutionNode->child;
        m_CurrentSolutionNode = m_CurrentSolutionNode->child;
        return &child->m_UserState;
      }
    }

    return nullptr;
  }

  // Get end node
  UserState* GetSolutionEnd()
  {
    m_CurrentSolutionNode = m_Goal;
    if ( m_Goal )
    {
      return &m_Goal->m_UserState;
    }

    return nullptr;
  }

  // Step solution iterator backwards
  UserState* GetSolutionPrev()
  {
    if ( m_CurrentSolutionNode )
    {
      if ( m_CurrentSolutionNode->parent )
      {
        Node* parent = m_CurrentSolutionNode->parent;
        m_CurrentSolutionNode = m_CurrentSolutionNode->parent;
        return &parent->m_UserState;
      }
    }

    return nullptr;
  }

  // For educational use and debugging it is useful to be able to view
  // the open and closed list at each step, here are two functions to allow that.

  UserState* GetOpenListStart()
  {
    float f, g, h;
    return GetOpenListStart( f, g, h );
  }

  UserState* GetOpenListStart( float& f, float& g, float& h )
  {
    iterDbgOpen = m_OpenList.begin();
    if ( iterDbgOpen != m_OpenList.end() )
    {
      f = ( *iterDbgOpen )->f;
      g = ( *iterDbgOpen )->g;
      h = ( *iterDbgOpen )->h;
      return &( *iterDbgOpen )->m_UserState;
    }

    return nullptr;
  }

  UserState* GetOpenListNext()
  {
    float f, g, h;
    return GetOpenListNext( f, g, h );
  }

  UserState* GetOpenListNext( float& f, float& g, float& h )
  {
    ++iterDbgOpen;
    if ( iterDbgOpen != m_OpenList.end() )
    {
      f = ( *iterDbgOpen )->f;
      g = ( *iterDbgOpen )->g;
      h = ( *iterDbgOpen )->h;
      return &( *iterDbgOpen )->m_UserState;
    }
    return nullptr;
  }

  UserState* GetClosedListStart()
  {
    float f, g, h;
    return GetClosedListStart( f, g, h );
  }

  UserState* GetClosedListStart( float& f, float& g, float& h )
  {
    iterDbgClosed = m_ClosedList.begin();
    if ( iterDbgClosed != m_ClosedList.end() )
    {
      f = ( *iterDbgClosed )->f;
      g = ( *iterDbgClosed )->g;
      h = ( *iterDbgClosed )->h;

      return &( *iterDbgClosed )->m_UserState;
    }

    return nullptr;
  }

  UserState* GetClosedListNext()
  {
    float f, g, h;
    return GetClosedListNext( f, g, h );
  }

  UserState* GetClosedListNext( float& f, float& g, float& h )
  {
    ++iterDbgClosed;
    if ( iterDbgClosed != m_ClosedList.end() )
    {
      f = ( *iterDbgClosed )->f;
      g = ( *iterDbgClosed )->g;
      h = ( *iterDbgClosed )->h;

      return &( *iterDbgClosed )->m_UserState;
    }

    return nullptr;
  }

  // Get the number of steps
  int GetStepCount() { return m_Steps; }

private:  // methods
  // This is called when a search fails or is cancelled to free all used
  // memory
  void FreeAllNodes()
  {
    // iterate open list and delete all nodes
    NodeVectorIterator iterOpen = m_OpenList.begin(), endOpen = m_OpenList.end();

    while ( iterOpen != endOpen )
    {
      Node* n = ( *iterOpen );
      FreeNode( n );

      ++iterOpen;
    }

    m_OpenList.clear();

    // iterate closed list and delete unused nodes
    NodeVectorIterator iterClosed, endClosed;

    for ( iterClosed = m_ClosedList.begin(), endClosed = m_ClosedList.end();
          iterClosed != endClosed; ++iterClosed )
    {
      Node* n = ( *iterClosed );
      FreeNode( n );
    }

    m_ClosedList.clear();

    FreeNode( m_Goal );  // goal is in no list
  }


  // This call is made by the search class when the search ends. A lot of nodes may be
  // created that are still present when the search ends. They will be deleted by this
  // routine once the search ends
  void FreeUnusedNodes()
  {
    // iterate open list and delete unused nodes
    NodeVectorIterator iterOpen = m_OpenList.begin(), endOpen = m_OpenList.end();

    while ( iterOpen != endOpen )
    {
      Node* n = ( *iterOpen );

      if ( !n->child )
      {
        FreeNode( n );
        n = nullptr;
      }
      ++iterOpen;
    }
    m_OpenList.clear();
    // iterate closed list and delete unused nodes
    NodeVectorIterator iterClosed, endClosed;

    for ( iterClosed = m_ClosedList.begin(), endClosed = m_ClosedList.end();
          iterClosed != endClosed; ++iterClosed )
    {
      Node* n = ( *iterClosed );

      if ( !n->child )
      {
        FreeNode( n );
        n = nullptr;
      }
    }
    m_ClosedList.clear();
  }

  // Node memory management
  Node* AllocateNode()
  {
#if !USE_FSA_MEMORY
    Node* p = new Node;
    return p;
#else
    Node* address = m_FixedSizeAllocator.alloc();

    if ( !address )
    {
      return nullptr;
    }
    m_AllocateNodeCount++;
    Node* p = new ( address ) Node;
    return p;
#endif
  }

  void FreeNode( Node* node )
  {
    m_FreeNodeCount++;

#if !USE_FSA_MEMORY
    delete node;
#else
    m_FixedSizeAllocator.free( node );
#endif
  }

private:  // data
  // Heap (simple vector but used as a heap, cf. Steve Rabin's game gems article)
  NodeVector m_OpenList;

  // Closed list is a vector.
  NodeVector m_ClosedList;

  NodeVector m_SolutionList;

  // Successors is a vector filled out by the user each type successors to a node
  // are generated
  NodeVector m_Successors;

  // State
  unsigned int m_State;

  // Counts steps
  int m_Steps;

  // Start and goal state pointers
  Node* m_Start;
  Node* m_Goal;

  Node* m_CurrentSolutionNode;

  // Memory
  Pol::Plib::FixedSizeAllocator<Node> m_FixedSizeAllocator;

  // Debug : need to keep these two iterators around
  // for the user Dbg functions
  NodeVectorIterator iterDbgOpen;
  NodeVectorIterator iterDbgClosed;

  // debugging : count memory allocation and free's
  int m_AllocateNodeCount;
  int m_FreeNodeCount;

  bool m_CancelRequest;
};
}  // namespace Pol::Plib


#endif  // defined STLASTAR_H
