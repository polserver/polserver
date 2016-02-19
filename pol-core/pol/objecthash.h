/** @file
 *
 * @par History
 */


#ifndef __OBJECTHASH_H
#define __OBJECTHASH_H

#include "reftypes.h"
#include "../clib/rawtypes.h"

#include <map>
#include <unordered_set>

namespace Pol
{
namespace Clib
{
class StreamWriter;
}

namespace Core
{
/*
template <class _Key> struct myhash { };
_STLP_TEMPLATE_NULL struct myhash<UObject*> {
size_t operator()(UObject* obj) const { return obj->serial; }
};
*/
class ObjectHash
{
public:
  typedef std::unordered_set<u32> ds;
  typedef std::pair<u32, UObjectRef> hashpair;
  typedef std::map<u32, UObjectRef> hs;
  typedef hs::iterator OH_iterator;
  typedef hs::const_iterator OH_const_iterator;

  ObjectHash();
  ~ObjectHash();

  bool Insert( UObject* obj );
  bool Remove( u32 serial );
  void Clear();
  void Reap();

  UObject* Find( u32 serial );
  u32 GetNextUnusedItemSerial();
  u32 GetNextUnusedCharSerial();
  void PrintContents( Clib::StreamWriter& sw ) const;

  hs::const_iterator begin() const;
  hs::const_iterator end() const;
  void ClearCharacterAccountReferences();

  ds::const_iterator dirty_deleted_begin() const;
  ds::const_iterator dirty_deleted_end() const;

  void CleanDeleted();
  void ClearDeleted();

  void RegisterCleanDeletedSerial( u32 serial );

private:
  hs hash;
  OH_iterator reap_iterator;

  ds dirty_deleted;
  ds clean_deleted;
};
}
}
#endif
