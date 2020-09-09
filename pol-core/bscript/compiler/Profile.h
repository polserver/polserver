#ifndef POLSERVER_PROFILE_H
#define POLSERVER_PROFILE_H

#include <atomic>

namespace Pol::Bscript::Compiler
{
class Profile
{
public:
  std::atomic<long long> build_workspace_micros;
  std::atomic<long long> register_const_declarations_micros;
  std::atomic<long long> optimize_micros;
  std::atomic<long long> disambiguate_micros;
  std::atomic<long long> analyze_micros;
  std::atomic<long long> codegen_micros;
  std::atomic<long long> prune_cache_select_micros;
  std::atomic<long long> prune_cache_delete_micros;

  std::atomic<long> ambiguities;

  std::atomic<long> parse_em_count;
  std::atomic<long> parse_src_count;

  std::atomic<long long> load_em_micros;
  std::atomic<long long> parse_em_micros;
  std::atomic<long long> ast_em_micros;

  std::atomic<long long> parse_src_micros;
  std::atomic<long long> ast_src_micros;

  std::atomic<long long> ast_inc_micros;

  std::atomic<long long> ast_resolve_functions_micros;

  std::atomic<long> cache_hits;
  std::atomic<long> cache_misses;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_PROFILE_H
