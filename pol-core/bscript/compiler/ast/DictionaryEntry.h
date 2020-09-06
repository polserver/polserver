#ifndef POLSERVER_DICTIONARYENTRY_H
#define POLSERVER_DICTIONARYENTRY_H

#include "compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class Expression;

class DictionaryEntry : public Node
{
public:
  DictionaryEntry( const SourceLocation&, std::unique_ptr<Expression> key,
                   std::unique_ptr<Expression> value );

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_DICTIONARYENTRY_H
