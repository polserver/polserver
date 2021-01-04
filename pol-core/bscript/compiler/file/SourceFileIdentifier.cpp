#include "SourceFileIdentifier.h"

#include <filesystem>

namespace Pol::Bscript::Compiler
{
SourceFileIdentifier::SourceFileIdentifier( unsigned index, std::string pathname )
    : index( index ),
      pathname( std::filesystem::path( std::move( pathname ) ).make_preferred().string() )
{
}

}  // namespace Pol::Bscript::Compiler
