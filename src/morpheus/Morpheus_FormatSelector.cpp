/**
 * Morpheus_FormatSelector.cpp
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

#include "morpheus/Morpheus_FormatSelector.hpp"
#include "morpheus/Morpheus_SparseMatrix.hpp"

#ifdef HPCG_WITH_MORPHEUS

#include "morpheus/Morpheus.hpp"
#ifdef HPCG_WITH_MULTI_FORMATS
#include "morpheus/Morpheus_SparseMatrixRoutines.hpp"
#include "morpheus/Morpheus_Parser.hpp"
#endif  // HPCG_WITH_MULTI_FORMATS

int GetFormat_Impl(const SparseMatrix &A, int default_matrix_fmt,
                   std::vector<format_id> &input_file) {
  int fmt_index = default_matrix_fmt;

#ifdef HPCG_WITH_MULTI_FORMATS
  if (input_file.size() == 0) return fmt_index;

  // select format based on the rank and coarse level
  for (size_t i = 0; i < input_file.size(); i++) {
    if (MorpheusSparseMatrixGetRank(A) == input_file[i].rank &&
        MorpheusSparseMatrixGetCoarseLevel(A) == input_file[i].mg_level) {
      fmt_index = input_file[i].format;
      break;
    }
  }
#endif  // HPCG_WITH_MULTI_FORMATS

  return fmt_index;
}

int GetLocalFormat(const SparseMatrix &A) {
#ifdef HPCG_WITH_MULTI_FORMATS
  std::vector<format_id> input_file = local_input_file;
#else
  std::vector<format_id> input_file = std::vector<format_id>();
#endif
  return GetFormat_Impl(A, local_matrix_fmt, input_file);
}

#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
int GetGhostFormat(const SparseMatrix &A) {
#ifdef HPCG_WITH_MULTI_FORMATS
  std::vector<format_id> input_file = ghost_input_file;
#else
  std::vector<format_id> input_file = std::vector<format_id>();
#endif
  return GetFormat_Impl(A, ghost_matrix_fmt, input_file);
}
#endif  // HPCG_WITH_SPLIT_DISTRIBUTED

#endif  // HPCG_WITH_MORPHEUS