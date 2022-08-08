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
#include "morpheus/Morpheus_SparseMatrix.hpp"
#include "morpheus/Morpheus_SparseMatrixRoutines.hpp"
#include "morpheus/Morpheus_Vector.hpp"
#include "morpheus/Morpheus_ExchangeHalo.hpp"
#include "morpheus/Morpheus_Timer.hpp"

#include "mytimer.hpp"

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
  double t_begin = morpheus_timer(), t0 = 0.0, tspmv = 0.0;
#ifndef HPCG_NO_MPI
  double thalo = 0.0;
  MTICK();
  MorpheusExchangeHalo(A, x);
  MTOCK(thalo);
#endif  // HPCG_NO_MPI

  using Vector_t          = HPCG_Morpheus_Vec<Morpheus::value_type>;
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;

  auto xv     = ((Vector_t*)x.optimizationData)->values.dev;
  auto yv     = ((Vector_t*)y.optimizationData)->values.dev;
  auto Alocal = Aopt->local.dev;
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
  using uvec  = typename Morpheus::UnmanagedVector<Morpheus::value_type>;
  auto Aghost = Aopt->ghost.dev;

  // wrap vector to local and ghost part
  auto xlocal = uvec(Alocal.nrows(), xv.data());
  auto xghost = uvec(Aghost.ncols(), xv.data() + Aghost.nrows());
  auto ywrap  = uvec(yv.size(), yv.data());

  double tlocal = 0.0, tghost = 0.0;
  MTICK();
  Morpheus::multiply<Morpheus::ExecSpace>(Alocal, xlocal, ywrap);
  Kokkos::fence();
  MTOCK(tlocal);

  MTICK();
  Morpheus::multiply<Morpheus::ExecSpace>(Aghost, xghost, ywrap, false);
  Kokkos::fence();
  MTOCK(tghost);
#else
  double tlocal = 0.0;
  MTICK();
  Morpheus::multiply<Morpheus::ExecSpace>(Alocal, xv, yv);
  Kokkos::fence();
  MTOCK(tlocal);
#endif

  tspmv = morpheus_timer() - t_begin;

#if defined(HPCG_WITH_MULTI_FORMATS)
  if (A.optimizationData != 0) {
    const int level = MorpheusSparseMatrixGetCoarseLevel(A);
    sub_mtimers[level].SPMV += tspmv;
    sub_mtimers[level].SPMV_LOCAL += tlocal;
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
    sub_mtimers[level].SPMV_GHOST += tghost;
#endif
#ifndef HPCG_NO_MPI
    sub_mtimers[level].HALO_SWAP += thalo;
#endif  // HPCG_NO_MPI
  }
#endif  // HPCG_WITH_MULTI_FORMATS

  return 0;
#else
  A.isSpmvOptimized = false;
  return ComputeSPMV_ref(A, x, y);
#endif  // HPCG_WITH_MORPHEUS
}
