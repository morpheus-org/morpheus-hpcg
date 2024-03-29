/**
 * Morpheus_VectorRoutines.cpp
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

#include "morpheus/Morpheus_VectorRoutines.hpp"
#include "morpheus/Morpheus_Vector.hpp"

#ifdef HPCG_WITH_MORPHEUS

void MorpheusInitializeVector(Vector& v) {
  v.optimizationData = new HPCG_Morpheus_Vec<Morpheus::value_type>();
}

void MorpheusOptimizeVector(Vector& v) {
  using mirror =
      typename Morpheus::UnmanagedVector<Morpheus::value_type>::HostMirror;
  using Vector_t = HPCG_Morpheus_Vec<Morpheus::value_type>;
  Vector_t* vopt = (Vector_t*)v.optimizationData;

  // Wrap host data around original hpcg vector data
  vopt->values.host = mirror(v.localLength, v.values);
  // Now send to device
  vopt->values.dev =
      Morpheus::create_mirror_container<Morpheus::Space>(vopt->values.host);
  Morpheus::copy(vopt->values.host, vopt->values.dev);
}

void MorpheusZeroVector(Vector& v) {
  using Vector_t = HPCG_Morpheus_Vec<Morpheus::value_type>;
  Vector_t* vopt = (Vector_t*)v.optimizationData;
  vopt->values.dev.assign(vopt->values.dev.size(), 0);  // Zero out x on device
}

#endif  // HPCG_WITH_MORPHEUS