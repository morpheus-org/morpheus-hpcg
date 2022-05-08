/**
 * ReadHpcgDat.cpp
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

#include "morpheus/ReadHpcgDat.hpp"

#if defined(HPCG_WITH_MORPHEUS) && defined(HPCG_WITH_MULTI_FORMATS)

struct formats_struct fmt_tuple;

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

void ReadMorpheusDat() {
  FILE *morpheusStream = fopen("Morpheus.dat", "r");

  if (!morpheusStream) return -1;

  int procid, lvlid, fmtid;
  int ctr = 0;
  do {
    // TODO: No fancy checks here yet.
    if (fscanf(morpheusStream, "%d", &procid) != 1) {
      break;
    }
    fscanf(morpheusStream, "%d", &lvlid);
    fscanf(morpheusStream, "%d", &fmtid);

    fmt_tuple.procid.push_back(procid);
    fmt_tuple.lvlid.push_back(lvlid);
    fmt_tuple.fmtid.push_back(fmtid);

    ctr++;
  } while (SkipUntilEol(morpheusStream) != EOF);

  fmt_tuple.nentries = ctr;

  fclose(morpheusStream);
}

#endif  // HPCG_WITH_MULTI_FORMATS