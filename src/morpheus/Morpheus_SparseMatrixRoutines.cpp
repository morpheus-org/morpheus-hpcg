/**
 * Morpheus_SparseMatrixRoutines.cpp
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

#include "morpheus/Morpheus_SparseMatrixRoutines.hpp"
#include "morpheus/Morpheus_SparseMatrix.hpp"
#include "morpheus/Morpheus_FormatSelector.hpp"

#ifdef HPCG_WITH_MORPHEUS

#ifndef HPCG_NO_MPI
#include <mpi.h>
#endif

void MorpheusInitializeSparseMatrix(SparseMatrix& A) {
  A.optimizationData = new HPCG_Morpheus_Mat();
}

#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
// TODO:: HpcgToMorpheusMatrix()
// TODO:: ReplaceMatrixDiagonal() for local & ghost parts
void HpcgToMorpheusMatrix(SparseMatrix& A) {
  //   // TODO: What is the localNumberOfNonzeros
  //   typename Morpheus::Csr::HostMirror Acsr(
  //       A.localNumberOfRows, A.localNumberOfColumns,
  //       A.localNumberOfNonzeros);

  //   Acsr.row_offsets(0) = 0;
  //   global_int_t k      = 0;

  //   for (local_int_t i = 0; i < A.localNumberOfRows; i++) {
  //     for (local_int_t j = 0; j < A.nonzerosInRow[i]; j++) {
  //       Acsr.column_indices(k) = A.mtxIndL[i][j];
  //       Acsr.values(k++)       = A.matrixValues[i][j];
  //     }

  //     Acsr.row_offsets(i + 1) = Acsr.row_offsets(i) + A.nonzerosInRow[i];
  //   }
  //   HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;
  //   Aopt->host              = Acsr;

  // #ifdef HPCG_WITH_MORPHEUS_DYNAMIC
  //   // In-place conversion w/ temporary allocation
  //   Morpheus::convert<Kokkos::Serial>(Aopt->values.host, GetFormat(A));
  // #endif
  //   // Now send to device
  //   Aopt->dev =
  //       Morpheus::create_mirror_container<Morpheus::Space>(Aopt->values.host);
  //   Morpheus::copy(Aopt->values.host, Aopt->values.dev);
}
#else
void HpcgToMorpheusMatrix(SparseMatrix& A) {
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
  Aopt->values.host       = Acsr;

#ifdef HPCG_WITH_MORPHEUS_DYNAMIC
  // In-place conversion w/ temporary allocation
  Morpheus::convert<Kokkos::Serial>(Aopt->values.host, GetFormat(A));
#endif
  // Now send to device
  Aopt->values.dev =
      Morpheus::create_mirror_container<Morpheus::Space>(Aopt->values.host);
  Morpheus::copy(Aopt->values.host, Aopt->values.dev);
}
#endif

void MorpheusOptimizeSparseMatrix(SparseMatrix& A) {
  HpcgToMorpheusMatrix(A);

#ifndef HPCG_NO_MPI
  using index_mirror = typename HPCG_Morpheus_Mat::IndexVector::HostMirror;
  using value_mirror = typename HPCG_Morpheus_Mat::ValueVector::HostMirror;

  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;

  // Handle buffer of elements to be send across processes
  Aopt->elementsToSend.host = index_mirror(A.totalToBeSent, A.elementsToSend);
  // Now send to device
  Aopt->elementsToSend.dev = Morpheus::create_mirror_container<Morpheus::Space>(
      Aopt->elementsToSend.host);
  Morpheus::copy(Aopt->elementsToSend.host, Aopt->elementsToSend.dev);

  // Handle send buffer of elements to be send across processes
  Aopt->sendBuffer.host = value_mirror(A.totalToBeSent, A.sendBuffer);
  // Now send to device
  Aopt->sendBuffer.dev =
      Morpheus::create_mirror_container<Morpheus::Space>(Aopt->sendBuffer.host);
  Morpheus::copy(Aopt->sendBuffer.host, Aopt->sendBuffer.dev);
#endif  // HPCG_NO_MPI
}

void MorpheusReplaceMatrixDiagonal(SparseMatrix& A, Vector& diagonal) {
  using mirror =
      typename Morpheus::UnmanagedVector<Morpheus::value_type>::HostMirror;
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;

  mirror diag(diagonal.localLength, diagonal.values);
  auto diag_dev = Morpheus::create_mirror_container<Morpheus::Space>(diag);

  Morpheus::copy(diag, diag_dev);
  Morpheus::update_diagonal<Morpheus::ExecSpace>(Aopt->values.dev, diag_dev);
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

#ifdef HPCG_WITH_MULTI_FORMATS
format_report MorpheusSparseMatrixGetProperties(const SparseMatrix& A) {
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;

  format_report entry;

  entry.id.rank     = MorpheusSparseMatrixGetRank(A);
  entry.id.mg_level = MorpheusSparseMatrixGetCoarseLevel(A);
  entry.id.format   = Aopt->values.dev.active_index();

  entry.nrows = Aopt->values.dev.nrows();
  entry.ncols = Aopt->values.dev.ncols();
  entry.nnnz  = Aopt->values.dev.nnnz();

  entry.memory = 0;
#ifdef HPCG_WITH_MORPHEUS_DYNAMIC
  if (Aopt->values.dev.active_enum() == Morpheus::COO_FORMAT) {
    entry.memory +=
        2 * Aopt->values.dev.nnnz() *
        ((double)sizeof(Morpheus::index_type));  // row_indices & column_indices
    entry.memory += Aopt->values.dev.nnnz() *
                    ((double)sizeof(Morpheus::value_type));  // values
  } else if (Aopt->values.dev.active_enum() == Morpheus::CSR_FORMAT) {
    entry.memory += (Aopt->values.dev.nrows() + 1) *
                    ((double)sizeof(Morpheus::index_type));  // row_offsets
    entry.memory += Aopt->values.dev.nnnz() *
                    ((double)sizeof(Morpheus::index_type));  // column_indices
    entry.memory += Aopt->values.dev.nnnz() *
                    ((double)sizeof(Morpheus::value_type));  // values
    // values
  } else if (Aopt->values.dev.active_enum() == Morpheus::DIA_FORMAT) {
    typename Morpheus::Dia Adia = Aopt->values.dev;
    entry.memory += Adia.ndiags() *
                    ((double)sizeof(Morpheus::index_type));  // diagonal_offsets
    entry.memory += (Adia.nrows() * Adia.ncols()) *
                    ((double)sizeof(Morpheus::value_type));  // values
  } else {
    throw Morpheus::RuntimeException("Selected invalid format.");
  }
#else
  entry.memory += (Aopt->values.dev.nrows() + 1) *
                  ((double)sizeof(Morpheus::index_type));  // row_offsets
  entry.memory += Aopt->values.dev.nnnz() *
                  ((double)sizeof(Morpheus::index_type));  // column_indices
  entry.memory += Aopt->values.dev.nnnz() *
                  ((double)sizeof(Morpheus::value_type));  // values
#endif  // HPCG_WITH_MORPHEUS_DYNAMIC

  return entry;
}
#endif  // HPCG_WITH_MULTI_FORMATS
#endif  // HPCG_WITH_MORPHEUS
