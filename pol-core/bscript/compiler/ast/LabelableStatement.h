#ifndef POLSERVER_LABELABLESTATEMENT_H
#define POLSERVER_LABELABLESTATEMENT_H

#include "Statement.h"

namespace Pol::Bscript::Compiler
{
class LabelableStatement : public Statement
{
public:
  LabelableStatement( const SourceLocation&, std::string label );

  const std::string& get_label() const { return label; }

private:
  friend class Disambiguator;
  void relabel( std::string new_label );
  std::string label;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_LABELABLESTATEMENT_H
