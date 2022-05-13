/**
 * Morpheus_MGDataRoutines.cpp
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

#include "morpheus/Morpheus_MGDataRoutines.hpp"

#ifdef HPCG_WITH_MORPHEUS

#include "morpheus/Morpheus_MGData.hpp"
#include "morpheus/Morpheus_VectorRoutines.hpp"

void MorpheusInitializeMGData(MGData& mg) {
  mg.optimizationData = new HPCG_Morpheus_MGData();

  MorpheusInitializeVector(*mg.rc);
  MorpheusInitializeVector(*mg.xc);
  MorpheusInitializeVector(*mg.Axf);
}

void MorpheusOptimizeMGData(MGData& mg) {
  using index_type_mirror =
      typename Morpheus::UnmanagedVector<local_int_t>::HostMirror;
  using MGData_t  = HPCG_Morpheus_MGData;
  MGData_t* MGopt = (MGData_t*)mg.optimizationData;

  MorpheusOptimizeVector(*mg.rc);
  MorpheusOptimizeVector(*mg.xc);
  MorpheusOptimizeVector(*mg.Axf);

  MGopt->f2c.host =
      index_type_mirror(mg.f2cOperator_localLength, mg.f2cOperator);
  MGopt->f2c.dev =
      Morpheus::create_mirror_container<Morpheus::Space>(MGopt->f2c.host);
  Morpheus::copy(MGopt->f2c.host, MGopt->f2c.dev);
}

#endif  // HPCG_WITH_MORPHEUS
