/** @file
 *
 * @par History
 * - 2009/09/05 Turley:    Added struct .? and .- as shortcut for .exists() and .erase()
 * - 2009/12/21 Turley:    ._method() call fix
 */


#ifndef BSCRIPT_BSTRUCT_H
#define BSCRIPT_BSTRUCT_H

#ifndef BSCRIPT_BOBJECT_H
#include "bobject.h"
#endif

#include <iosfwd>
#include <map>
#include <string>

#include "../clib/compilerspecifics.h"
#include "../clib/maputil.h"
#include "../clib/rawtypes.h"
#include "../clib/unicode.h"

namespace Pol
{
namespace Bscript
{
class ContIterator;
class Executor;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Bscript
{
class BStruct : public BObjectImp
{
public:
  BStruct();
  BStruct( std::istream& is, unsigned size, BObjectType type );
  static BObjectImp* unpack( std::istream& is );

  void addMember( const char* name, BObjectRef val );
  void addMember( const char* name, BObjectImp* imp );

  const BObjectImp* FindMember( const char* name );

  size_t mapcount() const;

  typedef std::map<std::string, BObjectRef, Clib::ci_cmp_pred> Contents;
  const Contents& contents() const;

protected:
  explicit BStruct( const BStruct& other, BObjectType type );
  explicit BStruct( BObjectType type );

  virtual BObjectImp* copy() const POL_OVERRIDE;

  virtual char packtype() const;
  virtual const char* typetag() const;
  virtual void FormatForStringRep( std::ostream& os, const std::string& key,
                                   const BObjectRef& bvalref ) const;

  virtual UnicodeString getStringRep() const POL_OVERRIDE;
  virtual size_t sizeEstimate() const POL_OVERRIDE;
  virtual void packonto( std::ostream& os ) const POL_OVERRIDE;
  virtual const char* typeOf() const POL_OVERRIDE;
  virtual u8 typeOfInt() const POL_OVERRIDE;

  virtual ContIterator* createIterator( BObject* pIterVal ) POL_OVERRIDE;

  virtual BObjectRef OperSubscript( const BObject& obj ) POL_OVERRIDE;
  virtual BObjectImp* call_method( const char* methodname, Executor& ex ) POL_OVERRIDE;
  virtual BObjectImp* call_method_id( const int id, Executor& ex,
                                      bool forcebuiltin = false ) POL_OVERRIDE;
  virtual BObjectRef set_member( const char* membername, BObjectImp* value,
                                 bool copy ) POL_OVERRIDE;
  virtual BObjectRef get_member( const char* membername ) POL_OVERRIDE;
  virtual BObjectRef operDotPlus( const char* name ) POL_OVERRIDE;
  virtual BObjectRef operDotMinus( const char* name ) POL_OVERRIDE;
  virtual BObjectRef operDotQMark( const char* name ) POL_OVERRIDE;
  virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy ) POL_OVERRIDE;

  friend class BStructIterator;

private:
  Contents contents_;
  BStruct& operator=( const BStruct& );  // not implemented
};
}
}
#endif
