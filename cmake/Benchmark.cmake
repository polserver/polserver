
if (ENABLE_BENCHMARK)
  message("* benchmark")
  include(FetchContent)
  set(BENCH_REPO "https://github.com/google/benchmark")
  set(BENCH_TAG "v1.9.4")
  set(FETCHCONTENT_QUIET OFF)
  set(FETCHCONTENT_BASE_DIR ${EXT_DOWNLOAD_DIR})
  set(BENCHMARK_ENABLE_TESTING OFF)
  FetchContent_Declare(
    google-benchmark
    GIT_REPOSITORY ${BENCH_REPO}
    GIT_TAG ${BENCH_TAG}
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(google-benchmark)
endif()
