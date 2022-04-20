
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

/*!
 @file ComputeSPMV.cpp

 HPCG routine
 */

#include "ComputeSPMV.hpp"

#ifdef HPCG_WITH_MORPHEUS
#include "MorpheusUtils.hpp"
#ifndef HPCG_NO_MPI
#include "mytimer.hpp"
#include "ExchangeHalo.hpp"
#endif  // HPCG_NO_MPI
#else
#include "ComputeSPMV_ref.hpp"
#endif  // HPCG_WITH_MORPHEUS

/*!
  Routine to compute sparse matrix vector product y = Ax where:
  Precondition: First call exchange_externals to get off-processor values of x

  This routine calls the reference SpMV implementation by default, but
  can be replaced by a custom, optimized routine suited for
  the target system.

  @param[in]  A the known system matrix
  @param[in]  x the known vector
  @param[out] y the On exit contains the result: Ax.

  @return returns 0 upon success and non-zero otherwise

  @see ComputeSPMV_ref
*/
int ComputeSPMV(const SparseMatrix& A, Vector& x, Vector& y) {
#ifdef HPCG_WITH_MORPHEUS
#ifndef HPCG_NO_MPI
  double t0      = 0.0;
  double t_begin = mytimer();
  MorpheusExchangeHalo(A, x);
  t0 = mytimer() - t_begin;
#endif  // HPCG_NO_MPI

  using Vector_t          = HPCG_Morpheus_Vec<Morpheus::value_type>;
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;
  Vector_t* xopt          = (Vector_t*)x.optimizationData;
  Vector_t* yopt          = (Vector_t*)y.optimizationData;
  Morpheus::multiply<Morpheus::ExecSpace>(Aopt->dev, xopt->dev, yopt->dev);

  Kokkos::fence();

#if !defined(HPCG_NO_MPI) && defined(HPCG_WITH_MULTI_FORMATS)
  if (A.optimizationData != 0) {
    int offset = MorpheusSparseMatrixGetCoarseLevel(A) * ntimers;
    sub_mtimers[offset + 3] += t0;  // Halo-Swap time
  }
#endif  // !HPCG_NO_MPI && HPCG_WITH_MULTI_FORMATS

  return 0;
#else
  A.isSpmvOptimized = false;
  return ComputeSPMV_ref(A, x, y);
#endif  // HPCG_WITH_MORPHEUS
}
