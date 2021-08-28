// History
//   2006/10/07 Shinigami: smaller Bug in Debug() (m_pFirst -> m_pFirstUsed)

/*

  FixedSizeAllocator class
  Copyright 2001 Justin Heyes-Jones

  This class is a constant time O(1) memory manager for objects of
  a specified type. The type is specified using a template class.

  Memory is allocated from a fixed size buffer which you can specify in the
  class constructor or use the default.

  Using GetFirst and GetNext it is possible to iterate through the elements
  one by one, and this would be the most common use for the class.

  I would suggest using this class when you want O(1) add and delete
  and you don't do much searching, which would be O(n). Structures such as binary
  trees can be used instead to get O(logn) access time.

*/

#ifndef FSA_H
#define FSA_H

#include "../clib/logfacility.h"
#include <stdio.h>
#include <string>
namespace Pol
{
namespace Plib
{
template <class USER_TYPE>
class FixedSizeAllocator
{
public:
  // Constants
  enum
  {
    FSA_DEFAULT_SIZE = 100
  };

  // This class enables us to transparently manage the extra data
  // needed to enable the user class to form part of the double-linked
  // list class
  struct FSA_ELEMENT
  {
    USER_TYPE UserType;

    FSA_ELEMENT* pPrev;
    FSA_ELEMENT* pNext;
  };

public:  // methods
  FixedSizeAllocator( size_t MaxElements = FSA_DEFAULT_SIZE )
      : m_pFirstUsed( nullptr ), m_MaxElements( MaxElements )
  {
    // Allocate enough memory for the maximum number of elements

    char* pMem = new char[m_MaxElements * sizeof( FSA_ELEMENT )];
    // Clear the memory
    memset( pMem, 0, sizeof( FSA_ELEMENT ) * m_MaxElements );

    m_pMemory = (FSA_ELEMENT*)pMem;

    // Set the free list first pointer
    m_pFirstFree = m_pMemory;

    // Point at first element
    FSA_ELEMENT* pElement = m_pFirstFree;

    // Set the double linked free list
    for ( unsigned int i = 0; i < m_MaxElements; i++ )
    {
      pElement->pPrev = pElement - 1;
      pElement->pNext = pElement + 1;

      pElement++;
    }

    // first element should have a null prev
    m_pFirstFree->pPrev = nullptr;
    // last element should have a null next
    ( pElement - 1 )->pNext = nullptr;
  }


  ~FixedSizeAllocator()
  {
    // Free up the memory
    delete[]( char* ) m_pMemory;
  }

  // Allocate a new USER_TYPE and return a pointer to it
  USER_TYPE* alloc()
  {
    FSA_ELEMENT* pNewNode = nullptr;

    if ( !m_pFirstFree )
    {
      return nullptr;
    }
    else
    {
      pNewNode = m_pFirstFree;
      m_pFirstFree = pNewNode->pNext;

      // if the new node points to another free node then
      // change that nodes prev free pointer...
      if ( pNewNode->pNext )
      {
        pNewNode->pNext->pPrev = nullptr;
      }

      // node is now on the used list

      pNewNode->pPrev = nullptr;  // the allocated node is always first in the list

      if ( m_pFirstUsed == nullptr )
      {
        pNewNode->pNext = nullptr;  // no other nodes
      }
      else
      {
        m_pFirstUsed->pPrev = pNewNode;  // insert this at the head of the used list
        pNewNode->pNext = m_pFirstUsed;
      }

      m_pFirstUsed = pNewNode;
    }

    return reinterpret_cast<USER_TYPE*>( pNewNode );
  }

  // Free the given user type
  // For efficiency I don't check whether the user_data is a valid
  // pointer that was allocated. I may add some debug only checking
  // (To add the debug check you'd need to make sure the pointer is in
  // the m_pMemory area and is pointing at the start of a node)
  void free( USER_TYPE* user_data )
  {
    FSA_ELEMENT* pNode = reinterpret_cast<FSA_ELEMENT*>( user_data );

    // manage used list, remove this node from it
    if ( pNode->pPrev )
    {
      pNode->pPrev->pNext = pNode->pNext;
    }
    else
    {
      // this handles the case that we delete the first node in the used list
      m_pFirstUsed = pNode->pNext;
    }

    if ( pNode->pNext )
    {
      pNode->pNext->pPrev = pNode->pPrev;
    }

    // add to free list
    if ( m_pFirstFree == nullptr )
    {
      // free list was empty
      m_pFirstFree = pNode;
      pNode->pPrev = nullptr;
      pNode->pNext = nullptr;
    }
    else
    {
      // Add this node at the start of the free list
      m_pFirstFree->pPrev = pNode;
      pNode->pNext = m_pFirstFree;
      m_pFirstFree = pNode;
    }
  }

  // For debugging this displays both lists (using the prev/next list pointers)
  void Debug()
  {
    INFO_PRINT << "free list";
    FSA_ELEMENT* p = m_pFirstFree;
    fmt::Writer _tmp;
    while ( p )
    {
      _tmp << fmt::hex( p->pPrev ) << "!" << fmt::hex( p->pNext ) << " ";
      p = p->pNext;
    }
    _tmp << "\n";
    INFO_PRINT << _tmp.str();

    INFO_PRINT << "used list\n";
    _tmp.Clear();

    p = m_pFirstUsed;
    while ( p )
    {
      _tmp << fmt::hex( p->pPrev ) << "!" << fmt::hex( p->pNext ) << " ";
      p = p->pNext;
    }
    _tmp << "\n";
    INFO_PRINT << _tmp.str();
  }

  // Iterators

  USER_TYPE* GetFirst() { return reinterpret_cast<USER_TYPE*>( m_pFirstUsed ); }
  USER_TYPE* GetNext( USER_TYPE* node )
  {
    return reinterpret_cast<USER_TYPE*>( ( reinterpret_cast<FSA_ELEMENT*>( node ) )->pNext );
  }

public:   // data
private:  // methods
private:  // data
  FSA_ELEMENT* m_pFirstFree;
  FSA_ELEMENT* m_pFirstUsed;
  size_t m_MaxElements;
  FSA_ELEMENT* m_pMemory;
};
}  // namespace Plib
}  // namespace Pol
#endif  // defined FSA_H
