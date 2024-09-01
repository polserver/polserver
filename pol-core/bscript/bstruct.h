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

#include "../clib/maputil.h"
#include "../clib/rawtypes.h"

namespace Pol
{
namespace Bscript
{
class ContIterator;
class BClassInstanceRef;
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

  virtual BObjectImp* copy() const override;

  virtual char packtype() const;
  virtual const char* typetag() const;
  virtual void FormatForStringRep( std::ostream& os, const std::string& key,
                                   const BObjectRef& bvalref ) const;

  virtual std::string getStringRep() const override;
  virtual size_t sizeEstimate() const override;
  virtual void packonto( std::ostream& os ) const override;
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;

  virtual ContIterator* createIterator( BObject* pIterVal ) override;

  virtual BObjectRef OperSubscript( const BObject& obj ) override;
  virtual BObjectImp* call_method( const char* methodname, Executor& ex ) override;
  virtual BObjectImp* call_method_id( const int id, Executor& ex,
                                      bool forcebuiltin = false ) override;
  virtual BObjectRef set_member( const char* membername, BObjectImp* value,
                                 bool copy ) override;
  virtual BObjectRef get_member( const char* membername ) override;
  virtual BObjectRef operDotPlus( const char* name ) override;
  virtual BObjectRef operDotMinus( const char* name ) override;
  virtual BObjectRef operDotQMark( const char* name ) override;
  virtual BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy ) override;

  friend class BStructIterator;
  friend class BClassInstanceRef;

private:
  Contents contents_;
  BStruct& operator=( const BStruct& );  // not implemented
};
}
}
#endif
