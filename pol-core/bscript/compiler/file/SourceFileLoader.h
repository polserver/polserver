#ifndef POLSERVER_SOURCEFILELOADER_H
#define POLSERVER_SOURCEFILELOADER_H

#include <string>

namespace Pol::Bscript::Compiler
{
class SourceFileLoader
{
public:
  virtual ~SourceFileLoader() = default;
  // Throws
  virtual std::string get_contents( const std::string& pathname ) const;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SOURCEFILELOADER_H
