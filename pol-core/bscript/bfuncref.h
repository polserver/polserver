#pragma once

#include "clib/refptr.h"
#include "bscript/bobject.h"
#include "bscript/bobjectimp.h"
#include "bscript/eprog.h"

#include <memory>
#include <utility>
#include <vector>

namespace Pol::Bscript
{
class Executor;
class BFunctionRef final : public BObjectImp
{
  using base = BObjectImp;

public:
  BFunctionRef( ref_ptr<EScriptProgram> program, u64 owner_id, unsigned function_reference_index,
                std::weak_ptr<ValueStackCont> globals, ValueStackCont&& captures );
  BFunctionRef( const BFunctionRef& B );

private:
  ~BFunctionRef() override = default;

public:
  size_t sizeEstimate() const override;
  bool validCall( const int id, Executor& ex, Instruction* inst ) const;
  bool validCall( const char* methodname, Executor& ex, Instruction* inst ) const;
  int numParams() const;
  unsigned pc() const;
  bool variadic() const;
  ref_ptr<EScriptProgram> prog() const;
  // Executor::instance_id() of the executor that built this reference, which may be gone.
  u64 owner_id() const;
  unsigned class_index() const;
  bool constructor() const;
  bool class_method() const;
  const std::vector<unsigned>& default_parameter_addresses() const;
  int default_parameter_count() const;

  // Does not consider variadic functions. Caller must handle variadic()
  // explicitly!
  std::pair<int /*min count*/, int /*max count*/> expected_args() const;

public:  // Class Machinery
  BObjectImp* copy() const override;
  bool isTrue() const override;
  bool operator==( const BObjectImp& objimp ) const override;
  BObjectImp* selfIsObjImp( const BObjectImp& ) const override;
  std::string getStringRep() const override;

  BObjectImp* call_method( const char* methodname, Executor& ex ) override;
  BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin = false ) override;

private:
  // Need to reference the program, not the Executor, as the exec that created
  // this funcref could be destroyed by the time the funcref gets called
  ref_ptr<EScriptProgram> prog_;
  u64 owner_id_;
  unsigned function_reference_index_;

public:
  std::weak_ptr<ValueStackCont> globals;
  ValueStackCont captures;
};
}  // namespace Pol::Bscript
