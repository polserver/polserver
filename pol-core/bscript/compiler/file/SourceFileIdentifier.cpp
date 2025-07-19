#include "SourceFileIdentifier.h"
#include "clib/filecont.h"

namespace Pol::Bscript::Compiler
{
SourceFileIdentifier::SourceFileIdentifier( unsigned index, std::string pathname )
    : index( index ), pathname( std::move( pathname ) ), _lines()
{
}

const std::vector<std::string>& SourceFileIdentifier::getLines() const
{
  if ( !_lines.empty() )
    return _lines;

  Clib::FileContents cont{ pathname.c_str(), true };
  std::string content{ cont.contents() };
  Clib::sanitizeUnicodeWithIso( &content );
  std::string::size_type pos = 0;
  std::string::size_type prev = 0;

  while ( ( pos = content.find_first_of( "\n\r", prev ) ) != std::string::npos )
  {
    auto strpos = pos;
    if ( content[pos] == '\r' && pos + 1 < content.size() && content[pos + 1] == '\n' )
      ++pos;  // windows lineendings..
    _lines.push_back( content.substr( prev, strpos - prev ) );
    prev = pos + 1;
  }

  if ( prev < content.length() )
    _lines.push_back( content.substr( prev ) );

  return _lines;
}
}  // namespace Pol::Bscript::Compiler
