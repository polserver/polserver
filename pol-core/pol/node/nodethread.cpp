/** @file
 *
 * @par History
 */

#ifdef HAVE_NODEJS

#include "node.h"
#include "nodethread.h"


namespace Pol
{
namespace Node
{
void start_node_thread()
{
  POLLOG_INFO << "Starting node thread";
  char *argv[] = {"node", "./main.js"}, argc = 2;
  node::Start( argc, argv );
  POLLOG_INFO << "Node thread finished";
}
}  // namespace Node
}  // namespace Pol

#else


namespace Pol
{
namespace Node
{
void start_node_thread()
{
  POLLOG_INFO << "Nodejs not compiled";
}
}  // namespace Node
}  // namespace Pol

#endif
