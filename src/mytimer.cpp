//@HEADER
// ***************************************************
//
// HPCG: High Performance Conjugate Gradient Benchmark
//
// Contact:
// Michael A. Heroux ( maherou@sandia.gov)
// Jack Dongarra     (dongarra@eecs.utk.edu)
// Piotr Luszczek    (luszczek@eecs.utk.edu)
//
// ***************************************************
//@HEADER

/* ************************************************************************
 * Modifications (c) 2022 The University of Edinburgh
 *
 * EPCC, The University of Edinburgh
 *
 * Contributing Authors:
 * Christodoulos Stylianou (c.stylianou@ed.ac.uk)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * ************************************************************************ */

/////////////////////////////////////////////////////////////////////////

// Function to return time in seconds.
// If compiled with no flags, return CPU time (user and system).
// If compiled with -DWALL, returns elapsed time.

/////////////////////////////////////////////////////////////////////////

#ifndef HPCG_NO_MPI
#include <mpi.h>

double mytimer(void) { return MPI_Wtime(); }

#elif !defined(HPCG_NO_OPENMP)
// If this routine is compiled with HPCG_NO_MPI defined and not compiled with
// HPCG_NO_OPENMP then use the OpenMP timer
#include <omp.h>
double mytimer(void) { return omp_get_wtime(); }
#else
#include <cstdlib>
#include <sys/time.h>
#include <sys/resource.h>
double mytimer(void) {
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
#endif  // HPCG_NO_MPI

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
#endif  // HPCG_WITH_MORPEHUS
