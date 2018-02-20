/** @file
 *
 * @par History
 */

#ifndef H_CHECKPNT
#define H_CHECKPNT
#include <string>

namespace Pol
{
namespace Core
{
void checkpoint( const char* msg, unsigned short minlvl = 11 );

class Checkpoint
{
public:
  explicit Checkpoint( const char* file );
  ~Checkpoint();

  void setvalue( unsigned line ) { _line = line; }
  void clear() { _line = 0; }

private:
  const char* _file;
  unsigned _line;
};
}

#define DECLARE_CHECKPOINT Core::Checkpoint checkpoint( __FILE__ )

#define UPDATE_CHECKPOINT() checkpoint.setvalue( __LINE__ )

#define CLEAR_CHECKPOINT() checkpoint.clear()
}
#endif
