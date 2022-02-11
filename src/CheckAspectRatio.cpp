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
 @file CheckAspectRatio.cpp

 HPCG routine
 */

#include <algorithm>

#ifndef HPCG_NO_MPI
#include <mpi.h>
#endif

#include "CheckAspectRatio.hpp"
#include "hpcg.hpp"

int CheckAspectRatio(double smallest_ratio, int x, int y, int z,
                     const char *what, bool DoIo) {
  double current_ratio =
      std::min(std::min(x, y), z) / double(std::max(std::max(x, y), z));

  if (current_ratio < smallest_ratio) {  // ratio of the smallest to the largest
    if (DoIo) {
      HPCG_fout << "The " << what << " sizes (" << x << "," << y << "," << z
                << ") are invalid because the ratio min(x,y,z)/max(x,y,z)="
                << current_ratio << " is too small (at least " << smallest_ratio
                << " is required)." << std::endl;
      HPCG_fout
          << "The shape should resemble a 3D cube. Please adjust and try again."
          << std::endl;
      HPCG_fout.flush();
    }

#ifndef HPCG_NO_MPI
    MPI_Abort(MPI_COMM_WORLD, 127);
#endif

    return 127;
  }

  return 0;
}
