
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
 @file GenerateProblem.cpp

 HPCG routine
 */

#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif

#include <cassert>
#include "GenerateCoarseProblem.hpp"
#include "GenerateGeometry.hpp"
#include "GenerateProblem.hpp"
#include "SetupHalo.hpp"

/*!
  Routine to construct a prolongation/restriction operator for a given fine grid
  matrix solution (as computed by a direct solver).

  @param[inout]  Af - The known system matrix, on output its coarse operator,
  fine-to-coarse operator and auxiliary vectors will be defined.

  Note that the matrix Af is considered const because the attributes we are
  modifying are declared as mutable.

*/

void GenerateCoarseProblem(const SparseMatrix &Af) {
  // Make local copies of geometry information.  Use global_int_t since the RHS
  // products in the calculations below may result in global range values.
  global_int_t nxf = Af.geom->nx;
  global_int_t nyf = Af.geom->ny;
  global_int_t nzf = Af.geom->nz;

  local_int_t nxc, nyc, nzc;  // Coarse nx, ny, nz
  assert(nxf % 2 == 0);
  assert(nyf % 2 == 0);
  assert(nzf % 2 == 0);  // Need fine grid dimensions to be divisible by 2
  nxc                      = nxf / 2;
  nyc                      = nyf / 2;
  nzc                      = nzf / 2;
  local_int_t *f2cOperator = new local_int_t[Af.localNumberOfRows];
  local_int_t localNumberOfRows =
      nxc * nyc * nzc;  // This is the size of our subblock
  // If this assert fails, it most likely means that the local_int_t is set to
  // int and should be set to long long
  assert(localNumberOfRows >
         0);  // Throw an exception of the number of rows is less than zero (can
              // happen if "int" overflows)

  // Use a parallel loop to do initial assignment:
  // distributes the physical placement of arrays of pointers across the memory
  // system
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
  for (local_int_t i = 0; i < localNumberOfRows; ++i) {
    f2cOperator[i] = 0;
  }

  // TODO:  This triply nested loop could be flattened or use nested parallelism
#ifndef HPCG_NO_OPENMP
#pragma omp parallel for
#endif
  for (local_int_t izc = 0; izc < nzc; ++izc) {
    local_int_t izf = 2 * izc;
    for (local_int_t iyc = 0; iyc < nyc; ++iyc) {
      local_int_t iyf = 2 * iyc;
      for (local_int_t ixc = 0; ixc < nxc; ++ixc) {
        local_int_t ixf               = 2 * ixc;
        local_int_t currentCoarseRow  = izc * nxc * nyc + iyc * nxc + ixc;
        local_int_t currentFineRow    = izf * nxf * nyf + iyf * nxf + ixf;
        f2cOperator[currentCoarseRow] = currentFineRow;
      }  // end iy loop
    }    // end even iz if statement
  }      // end iz loop

  // Construct the geometry and linear system
  Geometry *geomc = new Geometry;
  local_int_t zlc =
      0;  // Coarsen nz for the lower block in the z processor dimension
  local_int_t zuc =
      0;  // Coarsen nz for the upper block in the z processor dimension
  int pz = Af.geom->pz;
  if (pz > 0) {
    zlc = Af.geom->partz_nz[0] /
          2;  // Coarsen nz for the lower block in the z processor dimension
    zuc = Af.geom->partz_nz[1] /
          2;  // Coarsen nz for the upper block in the z processor dimension
  }
  GenerateGeometry(Af.geom->size, Af.geom->rank, Af.geom->numThreads,
                   Af.geom->pz, zlc, zuc, nxc, nyc, nzc, Af.geom->npx,
                   Af.geom->npy, Af.geom->npz, geomc);

  SparseMatrix *Ac = new SparseMatrix;
  InitializeSparseMatrix(*Ac, geomc);
  GenerateProblem(*Ac, 0, 0, 0);
  SetupHalo(*Ac);
  Vector *rc  = new Vector;
  Vector *xc  = new Vector;
  Vector *Axf = new Vector;
  InitializeVector(*rc, Ac->localNumberOfRows);
  InitializeVector(*xc, Ac->localNumberOfColumns);
  InitializeVector(*Axf, Af.localNumberOfColumns);
  Af.Ac          = Ac;
  MGData *mgData = new MGData;
  InitializeMGData(f2cOperator, rc, xc, Axf, *mgData);
#ifdef HPCG_WITH_MG
  mgData->f2cOperator_localLength = Af.localNumberOfRows;
#endif
  Af.mgData = mgData;

  return;
}
