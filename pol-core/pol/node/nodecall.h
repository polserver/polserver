/** @file
 *
 * @par History
 */


#ifndef NODECALL_H
#define NODECALL_H

#include "napi-wrap.h"
#include <future>

using namespace Napi;

namespace Pol
{
namespace Node
{
std::future<Napi::ObjectReference> require( const std::string& name );

std::future<bool> release( Napi::ObjectReference ref );

std::future<bool> call( Napi::ObjectReference& ref );

}  // namespace Node
}  // namespace Pol

#endif
