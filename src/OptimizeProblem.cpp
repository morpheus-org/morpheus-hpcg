
//@HEADER
// ***************************************************
//
// HPCG: High Performance Conjugate Gradient Benchmark
//
// Contact:
// Michael A. Heroux ( maherou@sandia.gov)
// Jack Dongarra     (dongarra@eecs.utk.edu)
// Piotr Luszczek    (luszczek@eecs.utk.edu)
//
// ***************************************************
//@HEADER

/* ************************************************************************
 * Modifications (c) 2022 The University of Edinburgh
 *
 * EPCC, The University of Edinburgh
 *
 * Contributing Authors:
 * Christodoulos Stylianou (c.stylianou@ed.ac.uk)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * ************************************************************************ */

/*!
 @file OptimizeProblem.cpp

 HPCG routine
 */

#include "OptimizeProblem.hpp"
#include "Geometry.hpp"

#ifdef HPCG_WITH_MORPHEUS
#include "morpheus/Morpheus_SparseMatrixRoutines.hpp"
#include "morpheus/Morpheus_VectorRoutines.hpp"
#include "morpheus/Morpheus_MGDataRoutines.hpp"
#include "morpheus/Morpheus_ReadHpcgDat.hpp"
#include "morpheus/Morpheus_Timer.hpp"

#if defined(HPCG_DEBUG)
#include "morpheus/Morpheus_IO.hpp"
#endif

std::vector<format_report> local_morpheus_report, local_sub_report;
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
std::vector<format_report> ghost_morpheus_report, ghost_sub_report;
#endif  // HPCG_WITH_SPLIT_DISTRIBUTED

#if defined(HPCG_WITH_MULTI_FORMATS)
std::vector<morpheus_timers> mtimers, sub_mtimers;
#endif  // HPCG_WITH_MULTI_FORMATS

#endif  // HPCG_WITH_MORPHEUS

#if defined(HPCG_USE_MULTICOLORING)
void multicolor(SparseMatrix& A);
#endif

/*!
  Optimizes the data structures used for CG iteration to increase the
  performance of the benchmark version of the preconditioned CG algorithm.

  @param[inout] A      The known system matrix, also contains the MG hierarchy
  in attributes Ac and mgData.
  @param[inout] data   The data structure with all necessary CG vectors
  preallocated
  @param[inout] b      The known right hand side vector
  @param[inout] x      The solution vector to be computed in future CG iteration
  @param[inout] xexact The exact solution vector

  @return returns 0 upon success and non-zero otherwise

  @see GenerateGeometry
  @see GenerateProblem
*/

int OptimizeProblem(SparseMatrix& A, CGData& data, Vector& b, Vector& x,
                    Vector& xexact) {
#ifdef HPCG_WITH_MORPHEUS
  MorpheusInitializeSparseMatrix(A);
  MorpheusSparseMatrixSetCoarseLevel(A, 0);
  MorpheusSparseMatrixSetRank(A);
  MorpheusOptimizeSparseMatrix(A);

#if defined(HPCG_DEBUG) || defined(HPCG_DETAILED_DEBUG)
  SparseMatrixWrite(A);
  MorpheusSparseMatrixWrite(A);
#endif

  local_sub_report.push_back(MorpheusSparseMatrixGetLocalProperties(A));
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
  ghost_sub_report.push_back(MorpheusSparseMatrixGetGhostProperties(A));
#endif  // HPCG_WITH_SPLIT_DISTRIBUTED

  int levels = 1;
  // Process all coarse level matrices
  SparseMatrix* M = A.Ac;
  while (M != 0) {
    MorpheusInitializeSparseMatrix(*M);
    MorpheusSparseMatrixSetCoarseLevel(*M, levels++);
    MorpheusSparseMatrixSetRank(*M);
    MorpheusOptimizeSparseMatrix(*M);

#if defined(HPCG_DEBUG) || defined(HPCG_DETAILED_DEBUG)
    int clvl = MorpheusSparseMatrixGetCoarseLevel(*M);
    SparseMatrixWrite(*M, "coarse-" + std::to_string(clvl) + "-");
    MorpheusSparseMatrixWrite(*M, "coarse-" + std::to_string(clvl) + "-");
#endif

    local_sub_report.push_back(MorpheusSparseMatrixGetLocalProperties(*M));
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
    ghost_sub_report.push_back(MorpheusSparseMatrixGetGhostProperties(*M));
#endif  // HPCG_WITH_SPLIT_DISTRIBUTED

    // Go to next level in hierarchy
    M = M->Ac;
  }

  M          = &A;
  MGData* mg = M->mgData;
  while (mg != 0) {
    M = M->Ac;
    MorpheusInitializeMGData(*mg);
    MorpheusOptimizeMGData(*mg);

    mg = M->mgData;
  }

#ifdef HPCG_WITH_MULTI_FORMATS
  // Local timers
  sub_mtimers.resize(levels);
  if (A.geom->rank == 0) {
    mtimers.resize(A.geom->size * levels);
  } else {
    mtimers.resize(0);
  }
#endif  // HPCG_WITH_MULTI_FORMATS

  if (A.geom->rank == 0) {
    local_morpheus_report.resize(A.geom->size * levels);
  } else {
    local_morpheus_report.resize(0);
  }

#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
  if (A.geom->rank == 0) {
    ghost_morpheus_report.resize(A.geom->size * levels);
  } else {
    ghost_morpheus_report.resize(0);
  }
#endif  // HPCG_WITH_SPLIT_DISTRIBUTED

  MorpheusInitializeVector(b);
  MorpheusInitializeVector(x);
  MorpheusInitializeVector(xexact);
  MorpheusOptimizeVector(b);
  MorpheusOptimizeVector(x);
  MorpheusOptimizeVector(xexact);

  MorpheusInitializeVector(data.r);
  MorpheusInitializeVector(data.z);
  MorpheusInitializeVector(data.p);
  MorpheusInitializeVector(data.Ap);
  MorpheusOptimizeVector(data.r);
  MorpheusOptimizeVector(data.z);
  MorpheusOptimizeVector(data.p);
  MorpheusOptimizeVector(data.Ap);
#endif  // HPCG_WITH_MORPHEUS

#if defined(HPCG_USE_MULTICOLORING)
  multicolor(A);
#endif  // HPCG_USE_MULTICOLORING

  return 0;
}

// Helper function (see OptimizeProblem.hpp for details)
double OptimizeProblemMemoryUse(const SparseMatrix& A) {
  double fnbytes = 0.0;
#ifdef HPCG_WITH_MORPHEUS
  fnbytes += MorpheusSparseMatrixGetLocalProperties(A).memory;
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
  fnbytes += MorpheusSparseMatrixGetLocalProperties(A).memory;
#endif
#endif  // HPCG_WITH_MORPHEUS

  return fnbytes;
}

#if defined(HPCG_USE_MULTICOLORING)
void multicolor(SparseMatrix& A) {
  const local_int_t nrow = A.localNumberOfRows;
  std::vector<local_int_t> colors(
      nrow, nrow);  // value `nrow' means `uninitialized'; initialized colors go
                    // from 0 to nrow-1
  int totalColors = 1;
  colors[0]       = 0;  // first point gets color 0

  // Finds colors in a greedy (a likely non-optimal) fashion.

  for (local_int_t i = 1; i < nrow; ++i) {
    if (colors[i] == nrow) {  // if color not assigned
      std::vector<int> assigned(totalColors, 0);
      int currentlyAssigned                      = 0;
      const local_int_t* const currentColIndices = A.mtxIndL[i];
      const int currentNumberOfNonzeros          = A.nonzerosInRow[i];

      for (int j = 0; j < currentNumberOfNonzeros; j++) {  // scan neighbors
        local_int_t curCol = currentColIndices[j];
        if (curCol < i) {  // if this point has an assigned color (points beyond
                           // `i' are unassigned)
          if (assigned[colors[curCol]] == 0) currentlyAssigned += 1;
          assigned[colors[curCol]] =
              1;  // this color has been used before by `curCol' point
        }         // else // could take advantage of indices being sorted
      }

      if (currentlyAssigned <
          totalColors) {  // if there is at least one color left to use
        for (int j = 0; j < totalColors; ++j)  // try all current colors
          if (assigned[j] == 0) {              // if no neighbor with this color
            colors[i] = j;
            break;
          }
      } else {
        if (colors[i] == nrow) {
          colors[i] = totalColors;
          totalColors += 1;
        }
      }
    }
  }

  std::vector<local_int_t> counters(totalColors);
  for (local_int_t i = 0; i < nrow; ++i) counters[colors[i]]++;

  // form in-place prefix scan
  local_int_t old = counters[0], old0;
  for (local_int_t i = 1; i < totalColors; ++i) {
    old0        = counters[i];
    counters[i] = counters[i - 1] + old;
    old         = old0;
  }
  counters[0] = 0;

  // translate `colors' into a permutation
  for (local_int_t i = 0; i < nrow; ++i)  // for each color `c'
    colors[i] = counters[colors[i]]++;
}
#endif  // HPCG_USE_MULTICOLORING
