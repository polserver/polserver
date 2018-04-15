#ifndef CLIB_FILECONT_H
#define CLIB_FILECONT_H

#include <string>
#include "unicode.h"

namespace Pol
{
namespace Clib
{
/**
 * Simple class to read a whole file as a utf8 string
 */
class FileContents
{
public:
  FileContents( const char* filname );

  const UnicodeString& contents() const;
  void set_contents( const UnicodeString& str );

private:
  UnicodeString contents_;
};
}
}
#endif
