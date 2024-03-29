/**
 * Morpheus_Vector.hpp
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

#ifndef HPCG_MORPHEUS_VECTOR_HPP
#define HPCG_MORPHEUS_VECTOR_HPP

#ifdef HPCG_WITH_MORPHEUS
#include "morpheus/Morpheus.hpp"  //local_int_t

namespace Morpheus {
// Morpheus Types
template <typename ValueType>
using Vector = Morpheus::DenseVector<ValueType, local_int_t, Space>;
template <typename ValueType>
using UnmanagedVector = Morpheus::DenseVector<ValueType, local_int_t, Space,
                                              Kokkos::MemoryUnmanaged>;
}  // namespace Morpheus

// Optimization data to be used by Vector
template <typename ValueType>
struct Morpheus_Vec_STRUCT {
  using type = Morpheus::Vector<ValueType>;
  Morpheus::Vector<ValueType> dev;
  typename Morpheus::Vector<ValueType>::HostMirror host;
};
template <typename ValueType>
using Morpheus_Vec = Morpheus_Vec_STRUCT<ValueType>;

template <typename ValueType>
struct HPCG_Morpheus_Vec_STRUCT {
  Morpheus_Vec<ValueType> values;
};

template <typename ValueType>
using HPCG_Morpheus_Vec = HPCG_Morpheus_Vec_STRUCT<ValueType>;

#endif  // HPCG_WITH_MORPHEUS
#endif  // HPCG_MORPHEUS_VECTOR_HPP
