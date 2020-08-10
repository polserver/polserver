#include "SourceFileIdentifier.h"

namespace Pol::Bscript::Compiler
{
SourceFileIdentifier::SourceFileIdentifier( unsigned index, std::string pathname )
  : index( index ), pathname( std::move( pathname ) )
{
}

SourceFileIdentifier::~SourceFileIdentifier() = default;

}  // namespace Pol::Bscript::Compiler
