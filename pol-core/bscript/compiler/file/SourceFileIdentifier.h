#ifndef POLSERVER_SOURCEFILEIDENTIFIER_H
#define POLSERVER_SOURCEFILEIDENTIFIER_H

#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class SourceFileIdentifier
{
public:
  SourceFileIdentifier( unsigned index, std::string pathname );

  const unsigned index;
  const std::string pathname;

  SourceFileIdentifier( const SourceFileIdentifier& ) = delete;
  SourceFileIdentifier& operator=( const SourceFileIdentifier& ) = delete;
  const std::vector<std::string>& getLines() const;

private:
  mutable std::vector<std::string> _lines;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SOURCEFILEIDENTIFIER_H
