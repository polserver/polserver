#include "ListingWriter.h"

#include <fmt/ostream.h>
#include <fstream>
#include <string_view>

#include "StoredToken.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/format/StoredTokenDecoder.h"
#include "bscript/compiler/representation/CompiledScript.h"

namespace
{
using namespace Pol::Bscript::Compiler;
using namespace std::string_view_literals;
struct SourceLinePrintInfo
{
  size_t instruction;
  size_t last_file;
  size_t last_line;
  const std::vector<DebugStore::InstructionInfo>& debug_instructions;
  const SourceFileIdentifiers& source_files;

  void print( std::ofstream& ofs )
  {
    if ( instruction >= debug_instructions.size() )
      return;
    const auto& debug = debug_instructions[instruction];
    if ( debug.file_index == last_file && debug.line_number == last_line )
      return;
    // -1 see InstructionEmitter::debug_file_line
    if ( debug.file_index - 1 >= source_files.size() )
      return;
    const auto& file = source_files[debug.file_index - 1];
    if ( file->pathname.ends_with( ".em" ) )
      return;
    last_file = debug.file_index;
    last_line = debug.line_number;
    SourceLocation loc{
        file.get(),
        { { .line_number = debug.line_number, .character_column = 1, .token_index = 0 },
          { .line_number = debug.line_number, .character_column = 1, .token_index = 0 } } };
    auto line = loc.getSourceLine();
    std::string_view lineview = line;
    lineview.remove_prefix( lineview.find_first_not_of( " \t"sv ) );
    fmt::print( ofs, "{}\n{}\n", loc, lineview );
  }
};
}  // namespace

namespace Pol::Bscript::Compiler
{
ListingWriter::ListingWriter( CompiledScript& compiled_script ) : compiled_script( compiled_script )
{
}

void ListingWriter::write( std::ofstream& ofs )
{
  StoredTokenDecoder decoder( compiled_script.module_descriptors, compiled_script.data );

  SourceLinePrintInfo info{ .instruction = 0,
                            .last_file = 0,
                            .last_line = 0,
                            .debug_instructions = compiled_script.debug.instructions,
                            .source_files = compiled_script.source_file_identifiers };

  for ( const auto& tkn : compiled_script.code )
  {
    std::string w;
    decoder.decode_to( tkn, w );
    info.print( ofs );
    fmt::print( ofs, "{}: {}\n", info.instruction, w );
    ++info.instruction;
  }
}

}  // namespace Pol::Bscript::Compiler
