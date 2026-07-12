#pragma once

#include "bobjectimp.h"

#include <string>
#include <vector>

namespace Pol::Bscript
{
class ObjArray final : public BObjectImp
{
public:
  using NameCont = std::vector<std::string>;
  using name_iterator = NameCont::iterator;
  using const_name_iterator = NameCont::const_iterator;

  using Cont = std::vector<BObjectRef>;
  using iterator = Cont::iterator;
  using const_iterator = Cont::const_iterator;

  NameCont name_arr;
  Cont ref_arr;

  ObjArray();
  ObjArray( const ObjArray& i );  // copy constructor

  void packonto( std::string& str ) const override;
  static BObjectImp* unpack( std::istream& is );
  size_t sizeEstimate() const override;
  BObjectImp* copy() const override;

  BObjectRef operDotPlus( const char* name ) override;
  long contains( const BObjectImp& imp ) const override;
  void operInsertInto( BObject& obj, const BObjectImp& objimp ) override;
  BObjectImp* selfPlusObjImp( const BObjectImp& other ) const override;
  BObjectImp* selfPlusObj( const BObjectImp& objimp ) const override;
  BObjectImp* selfPlusObj( const BLong& objimp ) const override;
  BObjectImp* selfPlusObj( const Double& objimp ) const override;
  BObjectImp* selfPlusObj( const String& objimp ) const override;
  BObjectImp* selfPlusObj( const ObjArray& objimp ) const override;
  void selfPlusObjImp( BObjectImp& objimp, BObject& obj ) override;
  void selfPlusObj( BObjectImp& objimp, BObject& obj ) override;
  void selfPlusObj( BLong& objimp, BObject& obj ) override;
  void selfPlusObj( Double& objimp, BObject& obj ) override;
  void selfPlusObj( String& objimp, BObject& obj ) override;
  void selfPlusObj( ObjArray& objimp, BObject& obj ) override;

  BObjectImp* call_method( const char* methodname, Executor& ex ) override;
  BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin = false ) override;
  BObjectRef set_member( const char* membername, BObjectImp* value, bool copy ) override;
  BObjectRef get_member( const char* membername ) override;

  void addElement( BObjectImp* imp );

  const BObjectImp* imp_at( unsigned index ) const;  // 1-based

  std::string getStringRep() const override;
  void printOn( std::ostream& os ) const override;

  BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy ) override;
  BObjectRef OperSubscript( const BObject& obj ) override;
  BObjectRef OperMultiSubscript( std::stack<BObjectRef>& indices ) override;

  bool operator==( const BObjectImp& objimp ) const override;

  ContIterator* createIterator( BObject* pIterVal ) override;

protected:
  explicit ObjArray( BObjectType type );
  void deepcopy();
};
}  // namespace Pol::Bscript
