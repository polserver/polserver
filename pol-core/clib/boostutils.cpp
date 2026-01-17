
#include "boostutils.h"

namespace Pol::boost_utils
{
flyweight_initializers::~flyweight_initializers()
{
#ifdef ENABLE_FLYWEIGHT_REPORT
  if ( Query::debug_flyweight_queries != nullptr )
    delete Query::debug_flyweight_queries;
  Query::debug_flyweight_queries = nullptr;
#endif
}

#ifdef ENABLE_FLYWEIGHT_REPORT
std::vector<bucket_query*>* Query::debug_flyweight_queries = nullptr;

bucket_query::bucket_query()
{
  Query::add( this );
}

void Query::add( bucket_query* b )
{
  if ( debug_flyweight_queries == nullptr )
    debug_flyweight_queries = new std::vector<bucket_query*>;
  debug_flyweight_queries->push_back( b );
}

std::vector<std::pair<size_t, size_t>> Query::getCountAndSize()
{
  std::vector<std::pair<size_t, size_t>> res;
  for ( const auto& factory : *debug_flyweight_queries )
  {
    res.emplace_back( factory->bucket_count(), factory->estimateSize() );
  }
  return res;
}
#endif
}  // namespace Pol::boost_utils
