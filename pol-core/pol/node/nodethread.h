/** @file
 *
 * @par History
 */


#ifndef NODETHREAD_H
#define NODETHREAD_H

#include "../bscript/eprog.h"
#include "napi-wrap.h"
#include <condition_variable>
#include <future>


namespace Pol
{
namespace Node
{
extern Napi::ObjectReference requireRef;
extern Napi::ThreadSafeFunction tsfn;

std::future<bool> start_node();

extern std::atomic<bool> running;

bool cleanup();
void triggerGC();

// void start_node_thread();
}  // namespace Node
}  // namespace Pol

#endif
