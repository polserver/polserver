#include "ListingWriter.h"

#include <fstream>

#include "StoredToken.h"
#include "bscript/compiler/format/StoredTokenDecoder.h"
#include "bscript/compiler/representation/CompiledScript.h"

namespace Pol::Bscript::Compiler
{
ListingWriter::ListingWriter( CompiledScript& compiled_script ) : compiled_script( compiled_script )
{
}

void ListingWriter::write( std::ofstream& ofs )
{
  StoredTokenDecoder decoder( compiled_script.module_descriptors, compiled_script.data );
  int i = 0;
  for ( auto& tkn : compiled_script.code )
  {
    std::string w;
    decoder.decode_to( tkn, w );

    ofs << i << ": " << w << "\n";
    ++i;
  }
}

}  // namespace Pol::Bscript::Compiler
