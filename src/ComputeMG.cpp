
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
 @file ComputeMG.cpp

 HPCG routine
 */

#include "ComputeMG.hpp"

#if defined(HPCG_WITH_MORPHEUS) && defined(HPCG_WITH_MG)
#include "Morpheus.hpp"
#include "MorpheusUtils.hpp"
#include "ComputeProlongation.hpp"
#include "ComputeRestriction.hpp"
#include "ComputeSPMV.hpp"
#include "ComputeSYMGS.hpp"
#else
#include "ComputeMG_ref.hpp"
#endif  // HPCG_WITH_MORPHEUS && HPCG_WITH_MG
/*!
  @param[in] A the known system matrix
  @param[in] r the input vector
  @param[inout] x On exit contains the result of the multigrid V-cycle with r as
  the RHS, x is the approximation to Ax = r.

  @return returns 0 upon success and non-zero otherwise

  @see ComputeMG
*/
int ComputeMG(const SparseMatrix& A, const Vector& r, Vector& x) {
#if defined(HPCG_WITH_MORPHEUS) && defined(HPCG_WITH_MG)
  A.isMgOptimized = true;

  assert(x.localLength ==
         A.localNumberOfColumns);  // Make sure x contain space for halo values

#ifdef HPCG_WITH_KOKKOS_CUDA
  using Vector_t = HPCG_Morpheus_Vec<Morpheus::value_type>;
  Vector_t* ropt = (Vector_t*)r.optimizationData;
  Vector_t* xopt = (Vector_t*)x.optimizationData;
#endif  // HPCG_WITH_KOKKOS_CUDA

  MorpheusZeroVector(x);  // initialize x to zero

  int ierr = 0;
  if (A.mgData != 0) {  // Go to next coarse level if defined
#ifdef HPCG_WITH_KOKKOS_CUDA
    Morpheus::copy(ropt->dev, ropt->host);
    Morpheus::copy(xopt->dev, xopt->host);
#endif  // HPCG_WITH_KOKKOS_CUDA
    int numberOfPresmootherSteps = A.mgData->numberOfPresmootherSteps;
    for (int i = 0; i < numberOfPresmootherSteps; ++i)
      ierr += ComputeSYMGS(A, r, x);
    if (ierr != 0) return ierr;
#ifdef HPCG_WITH_KOKKOS_CUDA
    Morpheus::copy(xopt->host, xopt->dev);
#endif  // HPCG_WITH_KOKKOS_CUDA

    ierr = ComputeSPMV(A, x, *A.mgData->Axf);
    if (ierr != 0) return ierr;

    // Perform restriction operation using simple injection
    ierr = ComputeRestriction(A, r);
    if (ierr != 0) return ierr;

    ierr = ComputeMG(*A.Ac, *A.mgData->rc, *A.mgData->xc);
    if (ierr != 0) return ierr;

    ierr = ComputeProlongation(A, x);
    if (ierr != 0) return ierr;

#ifdef HPCG_WITH_KOKKOS_CUDA
    Morpheus::copy(ropt->dev, ropt->host);
    Morpheus::copy(xopt->dev, xopt->host);
#endif  // HPCG_WITH_KOKKOS_CUDA
    int numberOfPostsmootherSteps = A.mgData->numberOfPostsmootherSteps;
    for (int i = 0; i < numberOfPostsmootherSteps; ++i)
      ierr += ComputeSYMGS(A, r, x);
    if (ierr != 0) return ierr;
#ifdef HPCG_WITH_KOKKOS_CUDA
    Morpheus::copy(xopt->host, xopt->dev);
#endif  // HPCG_WITH_KOKKOS_CUDA
  } else {
#ifdef HPCG_WITH_KOKKOS_CUDA
    // Copy r & x on host to perform SYMGS
    Morpheus::copy(ropt->dev, ropt->host);
    Morpheus::copy(xopt->dev, xopt->host);
#endif  // HPCG_WITH_KOKKOS_CUDA
    ierr = ComputeSYMGS(A, r, x);
    if (ierr != 0) return ierr;
#ifdef HPCG_WITH_KOKKOS_CUDA
    // Copy x back on device
    Morpheus::copy(xopt->host, xopt->dev);
#endif  // HPCG_WITH_KOKKOS_CUDA
  }
  return 0;

#else
  A.isMgOptimized = false;
  return ComputeMG_ref(A, r, x);
#endif  // HPCG_WITH_MORPHEUS && HPCG_WITH_MG
}
