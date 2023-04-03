/**
 * Morpheus_IO.cpp
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

#include "morpheus/Morpheus_IO.hpp"

#ifdef HPCG_WITH_MORPHEUS

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

void SparseMatrixWrite(SparseMatrix& A, std::string prefix) {
  std::stringstream external_entry, local_entry, entry;
  for (local_int_t i = 0; i < A.localNumberOfRows; i++) {
    for (int j = 0; j < A.nonzerosInRow[i]; j++) {
      global_int_t curIndex = A.mtxIndG[i][j];
      if (A.geom->rank == ComputeRankOfMatrixRow(*(A.geom), curIndex)) {
        local_entry << curIndex << " " << i << " " << A.mtxIndL[i][j] << " "
                    << A.matrixValues[i][j] << std::endl;
      } else {
        external_entry << curIndex << " " << i << " "
                       << A.mtxIndL[i][j] - A.localNumberOfRows << " "
                       << A.matrixValues[i][j] << std::endl;
      }
      entry << curIndex << " " << i << " " << A.mtxIndL[i][j] << " "
            << A.matrixValues[i][j] << std::endl;
    }
  }

  std::string external_filename =
      prefix + "external-matrix-" + std::to_string(A.geom->rank) + ".txt";
  std::ofstream external_out(external_filename);
  external_out << external_entry.str();

  std::string local_filename =
      prefix + "local-matrix-" + std::to_string(A.geom->rank) + ".txt";
  std::ofstream local_out(local_filename);
  local_out << local_entry.str();

  std::string filename =
      prefix + "matrix-" + std::to_string(A.geom->rank) + ".txt";
  std::ofstream out(filename);
  out << entry.str();
}

void MorpheusSparseMatrixWrite(SparseMatrix& A, std::string prefix) {
  HPCG_Morpheus_Mat* Aopt = (HPCG_Morpheus_Mat*)A.optimizationData;

  // Bring data to host first
  Morpheus::copy(Aopt->local.dev, Aopt->local.host);

  // Convert to CSR
  typename Morpheus::Csr::HostMirror Alocal;
  Morpheus::convert<Morpheus::Serial>(Aopt->local.host, Alocal);

  std::stringstream local_entry;
  for (size_t i = 0; i < Alocal.nrows(); i++) {
    for (local_int_t jj = Alocal.crow_offsets(i);
         jj < Alocal.crow_offsets(i + 1); jj++) {
      local_entry << i << " " << Alocal.ccolumn_indices(jj) << " "
                  << Alocal.cvalues(jj) << std::endl;
    }
  }
  std::string local_filename =
      prefix + "morpheus-local-matrix-" + std::to_string(A.geom->rank) + ".txt";
  std::ofstream local_out(local_filename);
  local_out << local_entry.str();

#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
  // Bring data to host first
  Morpheus::copy(Aopt->ghost.dev, Aopt->ghost.host);

  // Convert to CSR
  typename Morpheus::Csr::HostMirror Aghost;
  Morpheus::convert<Morpheus::Serial>(Aopt->ghost.host, Aghost);

  std::stringstream external_entry;
  for (size_t i = 0; i < Aghost.nrows(); i++) {
    for (local_int_t jj = Aghost.crow_offsets(i);
         jj < Aghost.crow_offsets(i + 1); jj++) {
      external_entry << i << " " << Aghost.ccolumn_indices(jj) << " "
                     << Aghost.cvalues(jj) << std::endl;
    }
  }
  std::string external_filename = prefix + "morpheus-external-matrix-" +
                                  std::to_string(A.geom->rank) + ".txt";
  std::ofstream external_out(external_filename);
  external_out << external_entry.str();
#endif
}

#endif  // HPCG_WITH_MORPHEUS
