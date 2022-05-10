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

#ifdef HPCG_WITH_MULTI_FORMATS
#include "morpheus/Morpheus_SparseMatrixRoutines.hpp"
#include "morpheus/Morpheus_ReadHpcgDat.hpp"
#endif  // HPCG_WITH_MULTI_FORMATS

int GetFormat(SparseMatrix &A) {
  int fmt_index = args.dynamic_format;

#ifdef HPCG_WITH_MULTI_FORMATS
  // select format based on the rank and coarse level
  for (int i = 0; i < fmt_tuple.nentries; i++) {
    if (MorpheusSparseMatrixGetRank(A) == fmt_tuple.procid[i] &&
        MorpheusSparseMatrixGetCoarseLevel(A) == fmt_tuple.lvlid[i]) {
      fmt_index = fmt_tuple.fmtid[i];
      break;
    }
  }
#endif  // HPCG_WITH_MULTI_FORMATS

  return fmt_index;
}

#endif  // HPCG_WITH_MORPHEUS