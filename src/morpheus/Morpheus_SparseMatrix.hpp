/**
 * Morpheus_SparseMatrix.hpp
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

#ifndef HPCG_MORPHEUS_SPARSEMATRIX_HPP
#define HPCG_MORPHEUS_SPARSEMATRIX_HPP

#ifdef HPCG_WITH_MORPHEUS

#include "morpheus/Morpheus.hpp"  //local_int_t
#include "morpheus/Morpheus_Vector.hpp"

namespace Morpheus {
using Csr = Morpheus::CsrMatrix<value_type, local_int_t, Space>;
using Dia = Morpheus::DiaMatrix<value_type, local_int_t, Space>;

#ifdef HPCG_WITH_MORPHEUS_DYNAMIC
using SparseMatrix = Morpheus::DynamicMatrix<value_type, local_int_t, Space>;
#else
using SparseMatrix = Csr;
#endif  // HPCG_WITH_MORPHEUS_DYNAMIC
}  // namespace Morpheus

struct Morpheus_Mat_STRUCT {
  Morpheus::SparseMatrix dev;
  typename Morpheus::SparseMatrix::HostMirror host;
};

typedef Morpheus_Mat_STRUCT Morpheus_Mat;

// Optimization data to be used by SparseMatrix
struct HPCG_Morpheus_Mat_STRUCT {
  Morpheus_Mat local;
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
  Morpheus_Mat ghost;
#endif

  int coarseLevel;
  local_int_t rank;
#ifndef HPCG_NO_MPI
  using IndexVector = typename Morpheus_Vec<local_int_t>::type;
  using ValueVector = typename Morpheus_Vec<Morpheus::value_type>::type;

  Morpheus_Vec<local_int_t> elementsToSend;
  Morpheus_Vec<Morpheus::value_type> sendBuffer;
#endif  // HPCG_NO_MPI
};

typedef HPCG_Morpheus_Mat_STRUCT HPCG_Morpheus_Mat;

#endif  // HPCG_WITH_MORPHEUS
#endif  // HPCG_MORPHEUS_SPARSEMATRIX_HPP
