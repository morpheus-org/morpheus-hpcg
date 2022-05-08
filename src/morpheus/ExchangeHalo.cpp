/**
 * ExchangeHalo.cpp
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

#include "morpheus/ExchangeHalo.hpp"

#ifdef HPCG_WITH_MORPHEUS

#ifndef HPCG_NO_MPI
#include <mpi.h>

#ifdef HPCG_WITH_KOKKOS_CUDA
// Needed for CUDA-aware check
#include "mpi-ext.h"
#endif  // HPCG_WITH_KOKKOS_CUDA

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
  sendBuffer = Aopt->sendBuffer.dev.data();
  xv         = xopt->values.dev.data();
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
  Morpheus::copy_by_key<Morpheus::ExecSpace>(
      Aopt->elementsToSend.dev, xopt->values.dev, Aopt->sendBuffer.dev);

#if !MPIX_CUDA_AWARE_SUPPORT
  Morpheus::copy(Aopt->sendBuffer.dev, Aopt->sendBuffer.host);
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
  Morpheus::copy(xopt->values.host, xopt->values.dev, localNumberOfRows,
                 localNumberOfRows + total_received);
#endif  // !MPIX_CUDA_AWARE_SUPPORT

  delete[] request;

  return;
}

#endif  // HPCG_NO_MPI
#endif  // HPCG_WITH_MORPHEUS
