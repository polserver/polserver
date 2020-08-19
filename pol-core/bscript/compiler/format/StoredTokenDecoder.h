#ifndef POLSERVER_STOREDTOKENDECODER_H
#define POLSERVER_STOREDTOKENDECODER_H

#include <vector>

#include "clib/formatfwd.h"

namespace Pol::Bscript
{
class StoredToken;
}

namespace Pol::Bscript::Compiler
{
class ModuleDescriptor;

class StoredTokenDecoder
{
public:
  StoredTokenDecoder( const std::vector<ModuleDescriptor>&, const std::vector<std::byte>& data );

  void decode_to( const StoredToken&, fmt::Writer& );

private:
  double double_at( unsigned offset ) const;

  const std::vector<ModuleDescriptor>& module_descriptors;
  const std::vector<std::byte>& data;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_STOREDTOKENDECODER_H
