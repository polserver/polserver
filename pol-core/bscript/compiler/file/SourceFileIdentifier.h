#ifndef POLSERVER_SOURCEFILEIDENTIFIER_H
#define POLSERVER_SOURCEFILEIDENTIFIER_H

#include <string>

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
class SourceFileIdentifier
{
public:
  SourceFileIdentifier( unsigned index, std::string pathname );
  ~SourceFileIdentifier();

  const unsigned index;
  const std::string pathname;

  SourceFileIdentifier( const SourceFileIdentifier& ) = delete;
  SourceFileIdentifier& operator=( const SourceFileIdentifier& ) = delete;
};

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol

#endif  // POLSERVER_SOURCEFILEIDENTIFIER_H
