#include "SourceFileIdentifier.h"

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
SourceFileIdentifier::SourceFileIdentifier( unsigned index, std::string pathname )
  : index( index ), pathname( std::move( pathname ) )
{
}

SourceFileIdentifier::~SourceFileIdentifier() = default;

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol
