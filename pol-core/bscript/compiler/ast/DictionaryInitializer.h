#ifndef POLSERVER_DICTIONARYINITIALIZER_H
#define POLSERVER_DICTIONARYINITIALIZER_H

#include "bscript/compiler/ast/Expression.h"

namespace Pol::Bscript::Compiler
{
class DictionaryEntry;

class DictionaryInitializer : public Expression
{
public:
  explicit DictionaryInitializer( const SourceLocation&,
                                  std::vector<std::unique_ptr<DictionaryEntry>> );

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_DICTIONARYINITIALIZER_H
