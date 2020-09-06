#ifndef POLSERVER_SOURCEFILEIDENTIFIER_H
#define POLSERVER_SOURCEFILEIDENTIFIER_H

#include <string>

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
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SOURCEFILEIDENTIFIER_H
