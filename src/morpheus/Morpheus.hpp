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
#include "Geometry.hpp"  //local_int_t
#include <vector>

// TODO: Move this in Morpheus Core
#define MORPHEUS_START_SCOPE() {  // Open Morpheus Scope
#define MORPHEUS_END_SCOPE() }    // Close Morpheus Scope

namespace Morpheus {
// Define Morpheus Execution and Memory Spaces
#if defined(HPCG_WITH_KOKKOS_SERIAL)
using ExecSpace = Kokkos::Serial;
using Space     = Kokkos::Serial;
#elif defined(HPCG_WITH_KOKKOS_OPENMP)
using ExecSpace = Kokkos::OpenMP;
using Space     = Kokkos::OpenMP;
#elif defined(HPCG_WITH_KOKKOS_CUDA)
using ExecSpace = Kokkos::Cuda;
using Space     = Kokkos::Cuda;
#endif

using value_type = double;
using index_type = local_int_t;
}  // namespace Morpheus

// used to hold any Morpheus related run-time arguments
// e.g dynamic_format
extern Morpheus::InitArguments args;

extern int local_matrix_fmt;
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
extern int ghost_matrix_fmt;
#endif

typedef struct format_id {
  global_int_t rank;
  int mg_level;
  int format;
} format_id;

typedef struct format_report {
  format_id id;
  local_int_t nrows;
  local_int_t ncols;
  global_int_t nnnz;
  double memory;
} format_report;

extern std::vector<format_report> local_morpheus_report, local_sub_report;
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
extern std::vector<format_report> ghost_morpheus_report, ghost_sub_report;
#endif  // HPCG_WITH_SPLIT_DISTRIBUTED

#ifdef HPCG_WITH_MULTI_FORMATS
extern std::vector<format_id> local_input_file;
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
extern std::vector<format_id> ghost_input_file;
#endif  // HPCG_WITH_SPLIT_DISTRIBUTED
#endif  // HPCG_WITH_MULTI_FORMATS

#endif  // HPCG_WITH_MORPHEUS
#endif  // HPCG_MORPHEUS_HPP
