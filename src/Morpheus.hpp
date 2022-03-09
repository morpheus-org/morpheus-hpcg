/**
 * Morpheus.hpp
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

#ifndef HPCG_MORPHEUS_HPP
#define HPCG_MORPHEUS_HPP

#ifdef HPCG_WITH_MORPHEUS
#include <Morpheus_Core.hpp>

// TODO: Move this in Morpheus Core
#define MORPHEUS_START_SCOPE() {  // Open Morpheus Scope
#define MORPHEUS_END_SCOPE() }    // Close Morpheus Scope

namespace Morpheus {
// Define Morpheus Execution and Memory Spaces
#if defined(HPCG_WITH_KOKKOS_SERIAL)
using ExecSpace = Kokkos::Serial;
using Space     = Kokkos::Serial;
#elif defined(HPCG_WITH_KOKKOS_OPENMP)
using ExecSpace    = Kokkos::OpenMP;
using Space        = Kokkos::OpenMP;
#elif defined(HPCG_WITH_KOKKOS_CUDA)
using ExecSpace = Kokkos::Cuda;
using Space     = Kokkos::Cuda;
#endif

using value_type = double;

// Morpheus Types
template <typename ValueType>
using Vector = Morpheus::DenseVector<ValueType, local_int_t, Space>;
template <typename ValueType>
using UnmanagedVector = Morpheus::DenseVector<ValueType, local_int_t, Space,
                                              Kokkos::MemoryUnmanaged>;

using Csr = Morpheus::CsrMatrix<value_type, local_int_t, Space>;
using Dia = Morpheus::DiaMatrix<value_type, local_int_t, Space>;

#ifdef HPCG_WITH_MORPHEUS_DYNAMIC
using SparseMatrix = Morpheus::DynamicMatrix<value_type, local_int_t, Space>;
#else
using SparseMatrix = Csr;
#endif

}  // namespace Morpheus
// used to hold any Morpheus related run-time arguments
// e.g dynamic_format
extern Morpheus::InitArguments args;

// Optimization data to be used by Vector
template <typename ValueType>
struct HPCG_Morpheus_Vec_STRUCT {
  Morpheus::Vector<ValueType> dev;
  typename Morpheus::Vector<ValueType>::HostMirror host;
};

template <typename ValueType>
using HPCG_Morpheus_Vec = HPCG_Morpheus_Vec_STRUCT<ValueType>;

// Optimization data to be used by SparseMatrix
struct HPCG_Morpheus_Mat_STRUCT {
  Morpheus::SparseMatrix dev;
  typename Morpheus::SparseMatrix::HostMirror host;
#ifndef HPCG_NO_MPI
  using IndexVector = Morpheus::Vector<local_int_t>;
  using ValueVector = Morpheus::Vector<Morpheus::value_type>;
  IndexVector elementsToSend_d;
  typename IndexVector::HostMirror elementsToSend_h;
  ValueVector sendBuffer_d;
  typename ValueVector::HostMirror sendBuffer_h;
#endif  // HPCG_NO_MPI
};

typedef HPCG_Morpheus_Mat_STRUCT HPCG_Morpheus_Mat;

#endif

#endif  // HPCG_MORPHEUS_HPP
