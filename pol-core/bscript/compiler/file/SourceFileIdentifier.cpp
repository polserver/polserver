#include "SourceFileIdentifier.h"

#include <filesystem>

namespace Pol::Bscript::Compiler
{
using path = std::filesystem::path;

SourceFileIdentifier::SourceFileIdentifier( unsigned index, std::string pathname )
    : index( index ), pathname( path( std::move( pathname ) ).make_preferred().string() )
{
}

}  // namespace Pol::Bscript::Compiler
