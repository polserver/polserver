#ifndef POLSERVER_STOREDTOKENDECODER_H
#define POLSERVER_STOREDTOKENDECODER_H

#include <string>
#include <vector>

#include "token.h"  // for DebugToken

namespace Pol::Bscript
{
class StoredToken;
}

namespace Pol::Bscript::Compiler
{
class DebugStore;
class ModuleDescriptor;

class StoredTokenDecoder
{
public:
  StoredTokenDecoder( const std::vector<ModuleDescriptor>&, const std::vector<std::byte>& data );

  void decode_to( const StoredToken&, std::string& );

private:
  double double_at( unsigned offset ) const;
  int integer_at( unsigned offset ) const;
  [[nodiscard]] uint16_t uint16_t_at( unsigned offset ) const;
  std::string string_at( unsigned offset ) const;
  const Pol::Bscript::DebugToken& debug_token_at( unsigned offset ) const;
  void decode_casejmp_table( std::string&, unsigned offset ) const;

  const std::vector<ModuleDescriptor>& module_descriptors;
  const std::vector<std::byte>& data;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_STOREDTOKENDECODER_H
