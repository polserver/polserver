#pragma once

#include "bstruct.h"

namespace Pol::Bscript
{
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

public:  // Class Machinery
  virtual BObjectImp* copy() const override;
  virtual bool isTrue() const override;

  virtual std::string getStringRep() const override;

private:
  ref_ptr<EScriptProgram> prog_;
  unsigned int index_;

public:
  std::shared_ptr<ValueStackCont> globals;
};

}  // namespace Pol::Bscript