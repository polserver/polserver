#include "DictionaryInitializer.h"

#include <format/format.h>

#include "bscript/compiler/ast/DictionaryEntry.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
DictionaryInitializer::DictionaryInitializer(
    const SourceLocation& source_location, std::vector<std::unique_ptr<DictionaryEntry>> entries )
    : Expression( source_location, std::move( entries ) )
{
}

void DictionaryInitializer::accept( NodeVisitor& visitor )
{
  visitor.visit_dictionary_initializer( *this );
}

void DictionaryInitializer::describe_to( fmt::Writer& w ) const
{
  w << "dictionary-initializer";
}

}  // namespace Pol::Bscript::Compiler
