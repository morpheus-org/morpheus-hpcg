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

#ifndef HPCG_NO_MPI
#include <mpi.h>

#ifdef HPCG_WITH_KOKKOS_CUDA
// Needed for CUDA-aware check
#include "mpi-ext.h"
#endif  // HPCG_WITH_KOKKOS_CUDA

#endif  // HPCG_NO_MPI

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
  // In-place conversion w/ temporary allocation
  Morpheus::convert<Kokkos::Serial>(Aopt->host, fmt_index);
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

void MorpheusSparseMatrixSetCoarseLevel(SparseMatrix& A, int level) {
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;
  Aopt->coarseLevel       = level;
}

void MorpheusSparseMatrixSetRank(SparseMatrix& A) {
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;
  Aopt->rank              = A.geom->rank;
}

int MorpheusSparseMatrixGetCoarseLevel(const SparseMatrix& A) {
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;
  return Aopt->coarseLevel;
}

int MorpheusSparseMatrixGetRank(const SparseMatrix& A) {
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;
  return Aopt->rank;
}

#ifdef HPCG_WITH_MG
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
#endif  // HPCG_WITH_MG

#ifndef HPCG_NO_MPI
void MorpheusExchangeHalo(const SparseMatrix& A, Vector& x) {
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;

  using Vector_t = HPCG_Morpheus_Vec<Morpheus::value_type>;
  Vector_t* xopt = (Vector_t*)x.optimizationData;

  local_int_t localNumberOfRows = A.localNumberOfRows;
  int num_neighbors             = A.numberOfSendNeighbors;
  local_int_t* receiveLength    = A.receiveLength;
  local_int_t* sendLength       = A.sendLength;
  int* neighbors                = A.neighbors;

  double *sendBuffer, *xv;

  // Extract Matrix pieces
#if MPIX_CUDA_AWARE_SUPPORT
  sendBuffer = Aopt->sendBuffer_d.data();
  xv         = xopt->dev.data();
#else
  sendBuffer = A.sendBuffer;
  xv         = x.values;
#endif  // MPIX_CUDA_AWARE_SUPPORT

  int size, rank;  // Number of MPI processes, My process ID
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  //
  //  first post receives, these are immediate receives
  //  Do not wait for result to come, will do that at the
  //  wait call below.
  //
  int MPI_MY_TAG = 99;

  MPI_Request* request = new MPI_Request[num_neighbors];

  //
  // Externals are at end of locals
  //
  double* x_external = (double*)xv + localNumberOfRows;
  // Post receives first
  for (int i = 0; i < num_neighbors; i++) {
    local_int_t n_recv = receiveLength[i];
    MPI_Irecv(x_external, n_recv, MPI_DOUBLE, neighbors[i], MPI_MY_TAG,
              MPI_COMM_WORLD, request + i);
    x_external += n_recv;
  }

  //
  // Fill up send buffer
  //
  Morpheus::copy_by_key<Morpheus::ExecSpace>(Aopt->elementsToSend_d, xopt->dev,
                                             Aopt->sendBuffer_d);

#if !MPIX_CUDA_AWARE_SUPPORT
  Morpheus::copy(Aopt->sendBuffer_d, Aopt->sendBuffer_h);
#endif

  //
  // Send to each neighbor
  //
  for (int i = 0; i < num_neighbors; i++) {
    local_int_t n_send = sendLength[i];
    MPI_Send(sendBuffer, n_send, MPI_DOUBLE, neighbors[i], MPI_MY_TAG,
             MPI_COMM_WORLD);
    sendBuffer += n_send;
  }

  //
  // Complete the reads issued above
  //
  MPI_Status status;
  for (int i = 0; i < num_neighbors; i++) {
    if (MPI_Wait(request + i, &status)) {
      std::exit(-1);  // TODO: have better error exit
    }
  }

#if !MPIX_CUDA_AWARE_SUPPORT
  // send received elements to device in one go
  using mirror = typename Morpheus::UnmanagedVector<local_int_t>::HostMirror;
  mirror elem_rec(num_neighbors, receiveLength);
  auto total_received = (num_neighbors > 0) ? Morpheus::reduce<Kokkos::Serial>(
                                                  elem_rec, num_neighbors)
                                            : 0;
  Morpheus::copy(xopt->host, xopt->dev, localNumberOfRows,
                 localNumberOfRows + total_received);
#endif  // !MPIX_CUDA_AWARE_SUPPORT

  delete[] request;

  return;
}
#endif  // HPCG_NO_MPI

#endif  // HPCG_WITH_MORPHEUS
