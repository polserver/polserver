#ifndef CLIB_UNIQUEFILE_H
#define CLIB_UNIQUEFILE_H

#include <cstdio>

namespace Pol::Clib
{
// RAII owner of a C FILE* handle: closes it on destruction, non-copyable. Implicitly
// converts to FILE* so existing fread/fseek/fprintf call sites read exactly as before.
// Default-constructs empty and reset()s to (re)assign the handle, so it serves both as
// a construct-once local (uoconvert's .cfg writers) and as a member opened after
// construction (the plib client-file caches).
class UniqueFile
{
public:
  UniqueFile() = default;
  explicit UniqueFile( FILE* fp ) : fp_( fp ) {}
  ~UniqueFile() { reset(); }

  UniqueFile( const UniqueFile& ) = delete;
  UniqueFile& operator=( const UniqueFile& ) = delete;

  // Close the current handle (if any) and take ownership of fp (nullptr to just close).
  void reset( FILE* fp = nullptr )
  {
    if ( fp_ )
      fclose( fp_ );
    fp_ = fp;
  }

  FILE* get() const { return fp_; }
  operator FILE*() const { return fp_; }

private:
  FILE* fp_ = nullptr;
};
}  // namespace Pol::Clib

#endif
