#pragma once

#include <map>
#include <set>

#include "bstruct.h"

namespace Pol::Bscript
{
class BFunctionRef;

class BClassInstance final : public BStruct
{
  typedef BStruct base;

public:
  BClassInstance( ref_ptr<EScriptProgram> program, int index,
                  std::shared_ptr<ValueStackCont> globals );
  BClassInstance( const BClassInstance& B );

private:
  ~BClassInstance() = default;

public:
  virtual size_t sizeEstimate() const override;
  ref_ptr<EScriptProgram> prog() const;
  unsigned index() const;
  // returns nullptr if no function found
  BFunctionRef* makeMethod( const char* method_name );

// Inherited from BStruct
  virtual const char* typetag() const override;

  std::set<unsigned> constructors_called;

public:  // Class Machinery
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual BObjectImp* copy() const override;
  virtual bool isTrue() const override;
  virtual BObjectImp* call_method( const char* methodname, Executor& ex ) override;
  virtual BObjectImp* call_method_id( const int id, Executor& ex,
                                      bool forcebuiltin = false ) override;
  virtual BObjectRef get_member_id( const int id ) override;
  virtual void packonto( std::ostream& os ) const override;

  virtual std::string getStringRep() const override;

private:
  ref_ptr<EScriptProgram> prog_;
  unsigned int index_;

public:
  std::shared_ptr<ValueStackCont> globals;
};

}  // namespace Pol::Bscript