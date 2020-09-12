#ifndef POLSERVER_SIDEBYSIDELISTINGWRITER_H
#define POLSERVER_SIDEBYSIDELISTINGWRITER_H

#include <map>
#include <string>
#include <vector>

#include "clib/formatfwd.h"

namespace Pol::Bscript
{
struct BSCRIPT_SECTION_HDR;
class StoredToken;
}

namespace Pol::Bscript::Compiler
{
class InputBuffer;
class ModuleDescriptor;

class SideBySideListingWriter
{
public:
  SideBySideListingWriter();
  ~SideBySideListingWriter();
  void disassemble_file( const std::string& ecl, const std::string& txt );
  void disassemble_section( InputBuffer& input, InputBuffer& section,
                            BSCRIPT_SECTION_HDR& section_header, fmt::Writer& tmp );

private:
  void disassemble_code_and_data( fmt::Writer& );

  std::vector<ModuleDescriptor> module_descriptors;

  unsigned code_offset = 0;
  std::vector<StoredToken> code;
  unsigned data_offset = 0;
  std::vector<std::byte> data;

};

void disassemble( const std::string& ecl, const std::string& txt );

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SIDEBYSIDELISTINGWRITER_H
