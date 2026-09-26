/** @file
 *
 * @par History
 */


#ifndef __SYMCONT_H
#define __SYMCONT_H

#include <cstdio>

#include "clib/rawtypes.h"

#ifndef POLSERVER_STOREDTOKEN_H
#include "bscript/StoredToken.h"
#endif


namespace Pol::Bscript
{
/// A block of a compiled script, read from its .ecl. The compiler writes .ecl files its own way,
/// so only the read side is here.
class SymbolContainer
{
protected:
  char* s;
  unsigned usedLen;
  unsigned allocLen;

public:
  SymbolContainer();
  virtual ~SymbolContainer();
  SymbolContainer( const SymbolContainer& ) = delete;
  SymbolContainer& operator=( const SymbolContainer& ) = delete;

  unsigned length() const { return usedLen; }
  // the whole buffer, not just the used part - what the container costs in memory
  unsigned allocated() const { return allocLen; }
  const char* array() const { return s; }
  void read( FILE* fp );
};

/* talk to these in statement numbers */
class StoredTokenContainer final : public SymbolContainer
{
public:
  unsigned count() const { return usedLen / sizeof( StoredToken ); }
  void atGet1( unsigned position, StoredToken& token ) const;
};
}  // namespace Pol::Bscript

#endif
