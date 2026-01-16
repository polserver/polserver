/** @file
 *
 * @par History
 */


#ifndef __TOKEN_H
#define __TOKEN_H

#ifndef __TOKENS_H
#include "tokens.h"
#endif
#ifndef __MODULES_H
#include "modules.h"
#endif

#include <iosfwd>
#include <set>

#include <fmt/ostream.h>

namespace Pol
{
namespace Bscript
{
using DebugToken = struct
{
  unsigned sourceFile;
  unsigned offset;
  unsigned strOffset;
};

class Token
{
public:
  BTokenId id;
  BTokenType type;
  double dval;
  union
  {
    int precedence;
    int sourceFile;
  };
  int dbg_filenum;
  int dbg_linenum;
  union
  {
    int lval;
    const unsigned char* dataptr;
  };
  bool deprecated;
  unsigned char module;

  static unsigned int instances();
  static void show_instances();

protected:
  std::string token;

public:
  const char* tokval() const { return token.c_str(); }
  Token();

  void nulStr();
  void setStr( std::string str );

  void printOn( std::ostream& outputStream ) const;
};

std::ostream& operator<<( std::ostream&, const Token& );
}  // namespace Bscript
namespace Clib
{
template <>
std::string tostring<Bscript::Token>( const Bscript::Token& v );
}
}  // namespace Pol
template <>
struct fmt::formatter<Pol::Bscript::Token> : fmt::ostream_formatter
{
};
#endif
