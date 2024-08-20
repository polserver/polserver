#include "bclassinstance.h"

namespace Pol::Bscript
{
BClassInstance::BClassInstance( ref_ptr<EScriptProgram> program, int index,
                                std::shared_ptr<ValueStackCont> globals )
    : BStruct( OTClassInstance ), prog_( program ), index_( index ), globals( std::move( globals ) )
{
}

BClassInstance::BClassInstance( const BClassInstance& B ) : BStruct( OTClassInstance )
{
  prog_ = B.prog_;
  index_ = B.index_;
  globals = B.globals;
}

size_t BClassInstance::sizeEstimate() const
{
  return sizeof( BClassInstance ) + prog_->sizeEstimate();
}

ref_ptr<EScriptProgram> BClassInstance::prog() const
{
  return prog_;
}

BObjectImp* BClassInstance::copy() const
{
  return new BClassInstance( *this );
}

bool BClassInstance::isTrue() const
{
  return true;
}

std::string BClassInstance::getStringRep() const
{
  auto class_name = prog_->symbols.array() + prog_->class_descriptors[index_].name_offset;
  return fmt::format( "<class {}>", class_name );
}
}  // namespace Pol::Bscript