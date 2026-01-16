#pragma once

#include <map>
#include <set>

#include "bstruct.h"

namespace Pol::Bscript
{
class BFunctionRef;

class BClassInstance final : public BStruct
{
  using base = BStruct;

public:
  BClassInstance( ref_ptr<EScriptProgram> program, int index,
                  std::shared_ptr<ValueStackCont> globals );
  BClassInstance( const BClassInstance& B );
  ~BClassInstance() override = default;

public:
  size_t sizeEstimate() const override;
  ref_ptr<EScriptProgram> prog() const;
  unsigned index() const;
  // returns nullptr if no function found
  BFunctionRef* makeMethod( const char* method_name );

  // Inherited from BStruct
  const char* typetag() const override;

  std::set<unsigned> constructors_called;

public:  // Class Machinery
  const char* typeOf() const override;
  u8 typeOfInt() const override;
  BObjectImp* copy() const override;
  bool isTrue() const override;
  BObjectImp* call_method( const char* methodname, Executor& ex ) override;
  BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin = false ) override;
  BObjectRef get_member_id( const int id ) override;
  void packonto( std::ostream& os ) const override;

  std::string getStringRep() const override;

private:
  ref_ptr<EScriptProgram> prog_;
  unsigned int index_;

public:
  std::shared_ptr<ValueStackCont> globals;
};

class BClassInstanceRef final : public BObjectImp
{
public:
  BClassInstanceRef( BClassInstance* );

private:
  ~BClassInstanceRef() override = default;

public:  // Class Machinery
  size_t sizeEstimate() const override;
  const char* typeOf() const override;
  u8 typeOfInt() const override;
  BObjectImp* copy() const override;
  bool isTrue() const override;
  BObjectImp* call_method( const char* methodname, Executor& ex ) override;
  BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin = false ) override;
  BObjectRef get_member_id( const int id ) override;

  std::string getStringRep() const override;

  ContIterator* createIterator( BObject* pIterVal ) override;

  BObjectRef OperSubscript( const BObject& obj ) override;
  BObjectRef set_member( const char* membername, BObjectImp* value, bool copy ) override;
  BObjectRef get_member( const char* membername ) override;
  BObjectRef operDotPlus( const char* name ) override;
  BObjectRef operDotMinus( const char* name ) override;
  BObjectRef operDotQMark( const char* name ) override;
  BObjectImp* array_assign( BObjectImp* idx, BObjectImp* target, bool copy ) override;

  inline BClassInstance* instance() const { return class_instance_.get(); }


private:
  ref_ptr<BClassInstance> class_instance_;
};


}  // namespace Pol::Bscript