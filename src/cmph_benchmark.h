#ifndef __CMPH_BENCHMARK_H__
#define __CMPH_BENCHMARK_H__

#if 0
#include <sys/time.h>
#include <sys/resource.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif
  
#define BM_REGISTER(func, iters) bm_register(#func, func, iters)
void bm_register(const char* name, void (*func)(int), int iters);
void run_benchmarks(int argc, const char** argv);

#ifdef __cplusplus
}
#endif

#endif  // __CMPH_BENCHMARK_H__
