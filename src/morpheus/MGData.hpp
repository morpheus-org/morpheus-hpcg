/**
 * MGData.hpp
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

#ifndef HPCG_MORPHEUS_MGDATA_HPP
#define HPCG_MORPHEUS_MGDATA_HPP

#ifdef HPCG_WITH_MORPHEUS
#include "SparseMatrix.hpp"

#include "morpheus/Morpheus.hpp"  //local_int_t
#include "morpheus/Vector.hpp"

#ifdef HPCG_WITH_MG
// Optimization data to be used by MG
struct HPCG_Morpheus_MGData_STRUCT {
  Morpheus_Vec<local_int_t> f2c;
};

typedef HPCG_Morpheus_MGData_STRUCT HPCG_Morpheus_MGData;

void MorpheusInitializeMGData(MGData& mg);
void MorpheusOptimizeMGData(MGData& mg);

#endif  // HPCG_WITH_MG
#endif  // HPCG_WITH_MORPHEUS
#endif  // HPCG_MORPHEUS_MGDATA_HPP
