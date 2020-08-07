#ifndef POLSERVER_LISTINGWRITER_H
#define POLSERVER_LISTINGWRITER_H

#include <iosfwd>

namespace Pol::Bscript::Compiler
{
class CompiledScript;

class ListingWriter
{
public:
  explicit ListingWriter( CompiledScript& );

  void write( std::ofstream& );

private:
  CompiledScript& compiled_script;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_LISTINGWRITER_H
