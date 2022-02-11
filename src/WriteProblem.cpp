
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
 @file WriteProblem.cpp

 HPCG routine
 */

#include <cstdio>
#include "WriteProblem.hpp"

/*!
  Routine to dump:
   - matrix in row, col, val format for analysis with MATLAB
   - x, xexact, b as simple arrays of numbers.

   Writes to A.dat, x.dat, xexact.dat and b.dat, respectivly.

   NOTE:  THIS CODE ONLY WORKS ON SINGLE PROCESSOR RUNS

   Read into MATLAB using:

       load A.dat
       A=spconvert(A);
       load x.dat
       load xexact.dat
       load b.dat

  @param[in] geom   The description of the problem's geometry.
  @param[in] A      The known system matrix
  @param[in] b      The known right hand side vector
  @param[in] x      The solution vector computed by CG iteration
  @param[in] xexact Generated exact solution

  @return Returns with -1 if used with more than one MPI process. Returns with 0
  otherwise.

  @see GenerateProblem
*/
int WriteProblem(const Geometry& geom, const SparseMatrix& A, const Vector b,
                 const Vector x, const Vector xexact) {
  if (geom.size != 1)
    return -1;  // TODO Only works on one processor.  Need better error handler
  const global_int_t nrow = A.totalNumberOfRows;

  FILE *fA = 0, *fx = 0, *fxexact = 0, *fb = 0;
  fA      = fopen("A.dat", "w");
  fx      = fopen("x.dat", "w");
  fxexact = fopen("xexact.dat", "w");
  fb      = fopen("b.dat", "w");

  if (!fA || !fx || !fxexact || !fb) {
    if (fb) fclose(fb);
    if (fxexact) fclose(fxexact);
    if (fx) fclose(fx);
    if (fA) fclose(fA);
    return -1;
  }

  for (global_int_t i = 0; i < nrow; i++) {
    const double* const currentRowValues        = A.matrixValues[i];
    const global_int_t* const currentRowIndices = A.mtxIndG[i];
    const int currentNumberOfNonzeros           = A.nonzerosInRow[i];
    for (int j = 0; j < currentNumberOfNonzeros; j++)
#ifdef HPCG_NO_LONG_LONG
      fprintf(fA, " %d %d %22.16e\n", i + 1,
              (global_int_t)(currentRowIndices[j] + 1), currentRowValues[j]);
#else
      fprintf(fA, " %lld %lld %22.16e\n", i + 1,
              (global_int_t)(currentRowIndices[j] + 1), currentRowValues[j]);
#endif
    fprintf(fx, "%22.16e\n", x.values[i]);
    fprintf(fxexact, "%22.16e\n", xexact.values[i]);
    fprintf(fb, "%22.16e\n", b.values[i]);
  }

  fclose(fA);
  fclose(fx);
  fclose(fxexact);
  fclose(fb);
  return 0;
}
