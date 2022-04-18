
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

#include "ComputeProlongation.hpp"

#if defined(HPCG_WITH_MORPHEUS) && defined(HPCG_WITH_MG)
#include "Morpheus.hpp"

#ifdef HPCG_WITH_KOKKOS_CUDA
template <unsigned int BLOCKSIZE, typename ValueType, typename IndexType>
__launch_bounds__(BLOCKSIZE) __global__
    void kernel_prolongation(const IndexType nc,
                             const ValueType* __restrict__ xc,
                             const IndexType* __restrict__ f2c,
                             ValueType* __restrict__ xf) {
  IndexType i = blockIdx.x * BLOCKSIZE + threadIdx.x;

  if (i >= nc) {
    return;
  }

  xf[f2c[i]] += xc[i];
}

template <typename ValueType, typename IndexType>
void Prolongation_Impl(const IndexType nc,
                       const Morpheus::Vector<ValueType>& xc,
                       const Morpheus::Vector<IndexType>& f2c,
                       Morpheus::Vector<ValueType>& xf) {
  const size_t BLOCK_SIZE = 256;
  const size_t NUM_BLOCKS = (nc - 1) / BLOCK_SIZE + 1;

  kernel_prolongation<BLOCK_SIZE, ValueType, IndexType>
      <<<NUM_BLOCKS, BLOCK_SIZE>>>(nc, xc.data(), f2c.data(), xf.data());
  Kokkos::fence();
}
#else
template <typename ValueType, typename IndexType>
void Prolongation_Impl(const IndexType nc,
                       const Morpheus::Vector<ValueType>& xc,
                       const Morpheus::Vector<IndexType>& f2c,
                       Morpheus::Vector<ValueType>& xf) {
#ifdef HPCG_ENABLE_KOKKOS_OPENMP
#pragma omp parallel for
#endif  // HPCG_ENABLE_KOKKOS_OPENMP
  for (IndexType i = 0; i < nc; ++i) {
    xf[f2c[i]] += xc[i];
  }
}
#endif  // HPCG_WITH_KOKKOS_CUDA
#else
#include "ComputeProlongation_ref.hpp"
#endif  // HPCG_WITH_MORPHEUS && HPCG_WITH_MG

/*!
  Routine to compute the coarse residual vector.

  @param[in]  Af - Fine grid sparse matrix object containing pointers to current
  coarse grid correction and the f2c operator.
  @param[inout] xf - Fine grid solution vector, update with coarse grid
  correction.

  Note that the fine grid residual is never explicitly constructed.
  We only compute it for the fine grid points that will be injected into
  corresponding coarse grid points.

  @return Returns zero on success and a non-zero value otherwise.
*/
int ComputeProlongation(const SparseMatrix& Af, Vector& xf) {
#if defined(HPCG_WITH_MORPHEUS) && defined(HPCG_WITH_MG)
  using Vector_t = HPCG_Morpheus_Vec<Morpheus::value_type>;
  using MGData_t = HPCG_Morpheus_MGData;

  MGData_t* MGopt = (MGData_t*)Af.mgData->optimizationData;
  Vector_t* xcopt = (Vector_t*)Af.mgData->xc->optimizationData;
  Vector_t* rcopt = (Vector_t*)Af.mgData->rc->optimizationData;
  Vector_t* xfopt = (Vector_t*)xf.optimizationData;

  Prolongation_Impl(rcopt->dev.size(), xcopt->dev, MGopt->f2c.dev, xfopt->dev);
#else
  ComputeProlongation_ref(Af, xf);
#endif  // HPCG_WITH_MORPHEUS && HPCG_WITH_MG
  return 0;
}
