/**
 * MorpheusUtils.cpp
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

#include "MorpheusUtils.hpp"
#include "Morpheus.hpp"

#ifdef HPCG_WITH_MORPHEUS

void MorpheusInitializeSparseMatrix(SparseMatrix& A) {
  A.optimizationData = new HPCG_Morpheus_Mat();
}

void MorpheusInitializeVector(Vector& v) {
  v.optimizationData = new HPCG_Morpheus_Vec<Morpheus::value_type>();
}

void MorpheusOptimizeSparseMatrix(SparseMatrix& A) {
  typename Morpheus::Csr::HostMirror Acsr(
      A.localNumberOfRows, A.localNumberOfColumns, A.localNumberOfNonzeros);

  Acsr.row_offsets(0) = 0;
  global_int_t k      = 0;

  for (local_int_t i = 0; i < A.localNumberOfRows; i++) {
    for (local_int_t j = 0; j < A.nonzerosInRow[i]; j++) {
      Acsr.column_indices(k) = A.mtxIndL[i][j];
      Acsr.values(k++)       = A.matrixValues[i][j];
    }

    Acsr.row_offsets(i + 1) = Acsr.row_offsets(i) + A.nonzerosInRow[i];
  }
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;
  Aopt->host              = Acsr;

#ifdef HPCG_WITH_MORPHEUS_DYNAMIC
  // In-place conversion w/ temporary allocation
  Morpheus::convert(Aopt->host, args.dynamic_format);
#endif
  // Now send to device
  Aopt->dev = Morpheus::create_mirror_container<Morpheus::Space>(Aopt->host);
  Morpheus::copy(Aopt->host, Aopt->dev);

#ifndef HPCG_NO_MPI
  using index_mirror = typename HPCG_Morpheus_Mat::IndexVector::HostMirror;
  using value_mirror = typename HPCG_Morpheus_Mat::ValueVector::HostMirror;

  // Handle buffer of elements to be send across processes
  Aopt->elementsToSend_h = index_mirror(A.totalToBeSent, A.elementsToSend);
  // Now send to device
  Aopt->elementsToSend_d = Morpheus::create_mirror_container<Morpheus::Space>(
      Aopt->elementsToSend_h);
  Morpheus::copy(Aopt->elementsToSend_h, Aopt->elementsToSend_d);

  // Handle send buffer of elements to be send across processes
  Aopt->sendBuffer_h = value_mirror(A.totalToBeSent, A.sendBuffer);
  // Now send to device
  Aopt->sendBuffer_d =
      Morpheus::create_mirror_container<Morpheus::Space>(Aopt->sendBuffer_h);
  Morpheus::copy(Aopt->sendBuffer_h, Aopt->sendBuffer_d);
#endif  // HPCG_NO_MPI
}

void MorpheusOptimizeVector(Vector& v) {
  using mirror =
      typename Morpheus::UnmanagedVector<Morpheus::value_type>::HostMirror;
  using Vector_t = HPCG_Morpheus_Vec<Morpheus::value_type>;
  Vector_t* vopt = (Vector_t*)v.optimizationData;

  // Wrap host data around original hpcg vector data
  vopt->host = mirror(v.localLength, v.values);
  // Now send to device
  vopt->dev = Morpheus::create_mirror_container<Morpheus::Space>(vopt->host);
  Morpheus::copy(vopt->host, vopt->dev);
}

void MorpheusZeroVector(Vector& v) {
  using Vector_t = HPCG_Morpheus_Vec<Morpheus::value_type>;
  Vector_t* vopt = (Vector_t*)v.optimizationData;
  vopt->dev.assign(vopt->dev.size(), 0);  // Zero out x on device
}

void MorpheusReplaceMatrixDiagonal(SparseMatrix& A, Vector& diagonal) {
  using mirror =
      typename Morpheus::UnmanagedVector<Morpheus::value_type>::HostMirror;
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;

  mirror diag(diagonal.localLength, diagonal.values);
  auto diag_dev = Morpheus::create_mirror_container<Morpheus::Space>(diag);

  Morpheus::copy(diag, diag_dev);
  Morpheus::update_diagonal<Morpheus::ExecSpace>(Aopt->dev, diag_dev);
}

#endif  // HPCG_WITH_MORPHEUS