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
void HpcgToMorpheusMatrix(SparseMatrix& A) {
  // Count nlocal & nexternal
  global_int_t nlocal = 0, nexternal = 0;
  for (local_int_t i = 0; i < A.localNumberOfRows; i++) {
    for (local_int_t j = 0; j < A.nonzerosInRow[i]; j++) {
      global_int_t curIndex = A.mtxIndG[i][j];
      if (A.geom->rank == ComputeRankOfMatrixRow(*(A.geom), curIndex)) {
        nlocal++;
      } else {
        nexternal++;
      }
    }
  }

  typename Morpheus::Csr::HostMirror Acsr(A.localNumberOfRows,
                                          A.localNumberOfRows, nlocal);
  typename Morpheus::Csr::HostMirror Acsr_ghost(
      A.localNumberOfRows, A.localNumberOfColumns - A.localNumberOfRows,
      nexternal);

  nlocal    = 0;
  nexternal = 0;

  Acsr.row_offsets(0)       = 0;
  Acsr_ghost.row_offsets(0) = 0;
  for (local_int_t i = 0; i < A.localNumberOfRows; i++) {
    for (local_int_t j = 0; j < A.nonzerosInRow[i]; j++) {
      global_int_t curIndex = A.mtxIndG[i][j];
      if (A.geom->rank == ComputeRankOfMatrixRow(*(A.geom), curIndex)) {
        Acsr.column_indices(nlocal) = A.mtxIndL[i][j];
        Acsr.values(nlocal++)       = A.matrixValues[i][j];
      } else {
        Acsr_ghost.column_indices(nexternal) =
            A.mtxIndL[i][j] - A.localNumberOfRows;
        Acsr_ghost.values(nexternal++) = A.matrixValues[i][j];
      }
    }
    Acsr.row_offsets(i + 1)       = nlocal;
    Acsr_ghost.row_offsets(i + 1) = nexternal;
  }

  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;
  Aopt->local.host        = Acsr;
  Aopt->ghost.host        = Acsr_ghost;

#ifdef HPCG_WITH_MORPHEUS_DYNAMIC
  // In-place conversion w/ temporary allocation
  Morpheus::convert<Kokkos::Serial>(Aopt->local.host, GetLocalFormat(A));
  Morpheus::convert<Kokkos::Serial>(Aopt->ghost.host, GetGhostFormat(A));
#endif
  // Now send to device
  Aopt->local.dev =
      Morpheus::create_mirror_container<Morpheus::Space>(Aopt->local.host);
  Morpheus::copy(Aopt->local.host, Aopt->local.dev);
  Aopt->ghost.dev =
      Morpheus::create_mirror_container<Morpheus::Space>(Aopt->ghost.host);
  Morpheus::copy(Aopt->ghost.host, Aopt->ghost.dev);
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
  Aopt->local.host        = Acsr;

#ifdef HPCG_WITH_MORPHEUS_DYNAMIC
  // In-place conversion w/ temporary allocation
  Morpheus::convert<Kokkos::Serial>(Aopt->local.host, GetLocalFormat(A));
#endif
  // Now send to device
  Aopt->local.dev =
      Morpheus::create_mirror_container<Morpheus::Space>(Aopt->local.host);
  Morpheus::copy(Aopt->local.host, Aopt->local.dev);
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
  Morpheus::update_diagonal<Morpheus::ExecSpace>(Aopt->local.dev, diag_dev);
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
template <typename MorpheusMatrix>
double count_memory(const MorpheusMatrix& A) {
  double memory     = 0;
  double index_size = (double)sizeof(Morpheus::index_type);
  double value_size = (double)sizeof(Morpheus::value_type);

#ifdef HPCG_WITH_MORPHEUS_DYNAMIC
  if (A.active_enum() == Morpheus::COO_FORMAT) {
    memory += 2 * A.nnnz() * index_size;  // row_indices & column_indices
    memory += A.nnnz() * value_size;      // values
  } else if (A.active_enum() == Morpheus::CSR_FORMAT) {
    memory += (A.nrows() + 1) * index_size;  // row_offsets
    memory += A.nnnz() * index_size;         // column_indices
    memory += A.nnnz() * value_size;         // values
    // values
  } else if (A.active_enum() == Morpheus::DIA_FORMAT) {
    typename Morpheus::Dia Adia = A;
    memory += Adia.ndiags() * index_size;                  // diagonal_offsets
    memory += (Adia.nrows() * Adia.ncols()) * value_size;  // values
  } else {
    throw Morpheus::RuntimeException("Selected invalid format.");
  }
#else
  memory += (A.nrows() + 1) * index_size;  // row_offsets
  memory += A.nnnz() * index_size;         // column_indices
  memory += A.nnnz() * value_size;         // values
#endif  // HPCG_WITH_MORPHEUS_DYNAMIC

  return memory;
}

format_report MorpheusSparseMatrixGetLocalProperties(const SparseMatrix& A) {
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;

  format_report entry;

  entry.id.rank     = MorpheusSparseMatrixGetRank(A);
  entry.id.mg_level = MorpheusSparseMatrixGetCoarseLevel(A);
  entry.id.format   = Aopt->local.dev.active_index();

  entry.nrows = Aopt->local.dev.nrows();
  entry.ncols = Aopt->local.dev.ncols();
  entry.nnnz  = Aopt->local.dev.nnnz();

  entry.memory = count_memory(Aopt->local.dev);

  return entry;
}
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
format_report MorpheusSparseMatrixGetGhostProperties(const SparseMatrix& A) {
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;

  format_report entry;

  entry.id.rank     = MorpheusSparseMatrixGetRank(A);
  entry.id.mg_level = MorpheusSparseMatrixGetCoarseLevel(A);
  entry.id.format   = Aopt->ghost.dev.active_index();

  entry.nrows = Aopt->ghost.dev.nrows();
  entry.ncols = Aopt->ghost.dev.ncols();
  entry.nnnz  = Aopt->ghost.dev.nnnz();

  entry.memory = count_memory(Aopt->ghost.dev);

  return entry;
}
#endif  // HPCG_WITH_SPLIT_DISTRIBUTED
#endif  // HPCG_WITH_MULTI_FORMATS
#endif  // HPCG_WITH_MORPHEUS
