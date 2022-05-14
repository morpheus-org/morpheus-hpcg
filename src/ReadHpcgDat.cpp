
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

#include <cstdio>
#include <iostream>
#include "ReadHpcgDat.hpp"

static int SkipUntilEol(FILE *stream) {
  int chOrEof;
  bool finished;

  do {
    chOrEof  = fgetc(stream);
    finished = (chOrEof == EOF) || (chOrEof == '\n') || (chOrEof == '\r');
  } while (!finished);

  if ('\r' == chOrEof) {  // on Windows, \r might be followed by \n
    int chOrEofExtra = fgetc(stream);

    if ('\n' == chOrEofExtra || EOF == chOrEofExtra)
      chOrEof = chOrEofExtra;
    else
      ungetc(chOrEofExtra, stream);
  }

  return chOrEof;
}

int ReadHpcgDat(int *localDimensions, int *secondsPerRun,
                int *localProcDimensions) {
  FILE *hpcgStream = fopen("hpcg.dat", "r");

  if (!hpcgStream) return -1;

  SkipUntilEol(hpcgStream);  // skip the first line

  SkipUntilEol(hpcgStream);  // skip the second line

  for (int i = 0; i < 3; ++i)
    if (fscanf(hpcgStream, "%d", localDimensions + i) != 1 ||
        localDimensions[i] < 16)
      localDimensions[i] = 16;

  SkipUntilEol(hpcgStream);  // skip the rest of the second line

  if (secondsPerRun !=
      0) {  // Only read number of seconds if the pointer is non-zero
    if (fscanf(hpcgStream, "%d", secondsPerRun) != 1 || secondsPerRun[0] < 0)
      secondsPerRun[0] = 30 * 60;  // 30 minutes
  }

  SkipUntilEol(hpcgStream);  // skip the rest of the third line

  for (int i = 0; i < 3; ++i)
    // the user didn't specify (or values are invalid) process dimensions
    if (fscanf(hpcgStream, "%d", localProcDimensions + i) != 1 ||
        localProcDimensions[i] < 1)
      localProcDimensions[i] =
          0;  // value 0 means: "not specified" and it will be fixed later

  fclose(hpcgStream);

  return 0;
}
