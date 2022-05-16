/**
 * Morpheus_Timer.cpp
 *
 * EPCC, The University of Edinburgh
 *
 * (c) 2022 The University of Edinburgh
 *
 * Contributing Authors:
 * Christodoulos Stylianou (c.stylianou@ed.ac.uk)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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