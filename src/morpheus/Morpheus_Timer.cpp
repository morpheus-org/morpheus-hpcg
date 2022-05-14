#include "morpheus/Morpheus_Timer.hpp"

#ifdef HPCG_WITH_MORPHEUS
#if defined(HPCG_WITH_KOKKOS_OPENMP)
#include <omp.h>
double morpheus_timer(void) { return omp_get_wtime(); }
#else

#include <cstdlib>
#include <sys/time.h>
#include <sys/resource.h>
double morpheus_timer(void) {
  struct timeval tp;
  static long start = 0, startu;
  if (!start) {
    gettimeofday(&tp, NULL);
    start  = tp.tv_sec;
    startu = tp.tv_usec;
    return 0.0;
  }
  gettimeofday(&tp, NULL);
  return ((double)(tp.tv_sec - start)) + (tp.tv_usec - startu) / 1000000.0;
}

#endif  // HPCG_WITH_KOKKOS_OPENMP
#endif  // HPCG_WITH_MORPHEUS