#include "ListingWriter.h"

#include <fstream>

#include "compiler/representation/CompiledScript.h"
#include "StoredToken.h"
#include "StoredTokenDecoder.h"

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
    fmt::Writer w;
    decoder.decode_to(tkn, w);

    ofs << i << ": " << w.str() << "\n";
    ++i;
  }
}

}  // namespace Pol::Bscript::Compiler