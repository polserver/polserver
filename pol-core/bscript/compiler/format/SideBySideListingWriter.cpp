#include "SideBySideListingWriter.h"

#include <map>
#include <format/format.h>

#include "clib/logfacility.h"

#include "StoredToken.h"
#include "filefmt.h"
#include "compiler/format/InputBuffer.h"
#include "compiler/format/StoredTokenDecoder.h"
#include "compiler/representation/ModuleDescriptor.h"
#include "compiler/representation/ModuleFunctionDescriptor.h"

using namespace Pol::Bscript;
using namespace Pol::Bscript::Compiler;

namespace Pol::Bscript::Compiler
{
SideBySideListingWriter::SideBySideListingWriter() = default;
SideBySideListingWriter::~SideBySideListingWriter() = default;

void SideBySideListingWriter::disassemble_section( InputBuffer& input, InputBuffer& section,
                                        BSCRIPT_SECTION_HDR& section_header, fmt::Writer& tmp )
{
  switch ( section_header.type )
  {
  case BSCRIPT_SECTION_MODULE:
  {  // the module section data length is 0 sigh
    tmp << fmt::pad( fmt::hex( input.offset() ), 5, '0' ) << "  : Module section\n";
    auto module_hdr = input.read<BSCRIPT_MODULE_HDR>();
    tmp << "  Name: " << module_hdr.modulename << "\n";
    tmp << "  Functions: " << module_hdr.nfuncs << "\n";
    std::vector<ModuleFunctionDescriptor> function_descriptors;

    for ( unsigned i = 0; i < module_hdr.nfuncs; ++i )
    {
      tmp << fmt::pad( fmt::hex( input.offset() ), 5, '0' ) << "  : ";
      auto module_function = input.read<BSCRIPT_MODULE_FUNCTION>();
      tmp << module_function.funcname << "(" << module_function.nargs << " arguments)\n";
      function_descriptors.emplace_back( module_function.funcname, module_function.nargs );
    }

    module_descriptors.emplace_back( module_hdr.modulename, std::move( function_descriptors ) );
  }
  break;
  case BSCRIPT_SECTION_PROGDEF:
    break;
  case BSCRIPT_SECTION_CODE:
  {
    tmp << fmt::pad( fmt::hex( section.offset() ), 5, '0' ) << "  : Code section\n";
    auto code_length = section.read<unsigned>();
    tmp << "  Length: " << code_length << "\n";
    code_offset = section.offset();
    while ( section.more() )
    {
      code.push_back( section.read<StoredToken>() );
    }
  }
  break;
  case BSCRIPT_SECTION_SYMBOLS:
  {
    tmp << fmt::pad( fmt::hex( section.offset() ), 5, '0' ) << "  : Data section\n";
    auto data_length = section.read<unsigned>();
    data_offset = section.offset();
    data = section.read_buffer( data_length ).get_buffer();

    // now that we have both code and data, we can disassemble both
    disassemble_code_and_data( tmp );
  }
  break;
  case BSCRIPT_SECTION_EXPORTED_FUNCTIONS:
  {
    tmp << fmt::pad( fmt::hex( section.offset() ), 5, '0' ) << "  : Exported functions\n";
    while ( section.more() )
    {
      auto offset = section.offset();
      auto ef = section.read<BSCRIPT_EXPORTED_FUNCTION>();
      tmp << fmt::pad( fmt::hex( offset ), 5, '0' ) << "  :  " << ef.funcname << " PC=" << ef.PC
          << " parameters: " << ef.nargs << "\n";
    }
    break;
  }
  }
}
void SideBySideListingWriter::disassemble_file( const std::string& ecl, const std::string& txt )
{
  auto input = InputBuffer::read( ecl );

  std::map<std::string, unsigned> data_labels;

  fmt::Writer tmp;

  tmp << fmt::pad( fmt::hex( input.offset() ), 5, '0' ) << "  "
      << "FILE HEADER\n";
  auto header = input.read<BSCRIPT_FILE_HDR>();
  auto blank = "       ";
  tmp << blank << "  Magic: " << header.magic2[0] << header.magic2[1] << "\n"
      << blank << "  Version: " << header.version << "\n"
      << blank << "  Globals: " << header.globals << "\n";

  while ( input.more() )
  {
    auto section_offset = input.offset();

    auto section_header = input.read<BSCRIPT_SECTION_HDR>();
    tmp << fmt::pad( fmt::hex( section_offset ), 5, '0' ) << "  : Section\n";
    tmp << blank << "  Type: " << section_header.type << "\n"
        << blank << "  Length: " << section_header.length << "\n";

    auto section = input.read_buffer( section_header.length );

    disassemble_section( input, section, section_header, tmp );
  }

  std::ofstream ofs( txt );
  ofs << tmp.c_str();
  INFO_PRINT << "wrote " << txt << "\n";
}

void SideBySideListingWriter::disassemble_code_and_data( fmt::Writer& tmp )
{
  tmp << "Code:\n";
  unsigned offset = code_offset;
  unsigned PC = 0;
  StoredTokenDecoder decoder( module_descriptors, data, nullptr );
  for ( auto& stored_token : code )
  {
    tmp << fmt::pad( fmt::hex( offset ), 5, '0' ) << "  ";
    // bytes
    const auto* ins_data = reinterpret_cast<const unsigned char*>( &stored_token );
    for ( unsigned i = 0; i < sizeof stored_token; ++i )
    {
      tmp << fmt::pad( fmt::hex( ins_data[i] ), 2, '0' ) << " ";
    }
    tmp << " ";
    tmp << fmt::pad( PC, 5, ' ' ) << ": ";
    decoder.decode_to( stored_token, tmp );
    tmp << "\n";

    offset += sizeof stored_token;
    PC++;
  }
}

}  // namespace Pol::Bscript::Compiler
