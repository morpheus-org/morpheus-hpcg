/**
 * Morpheus_ReadHpcgDat.cpp
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

#include "morpheus/Morpheus_ReadHpcgDat.hpp"

#if defined(HPCG_WITH_MORPHEUS)
#if defined(HPCG_WITH_MULTI_FORMATS)

#include "morpheus/Morpheus.hpp"

std::vector<format_id> input_file;

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

int ReadMorpheusDat(std::string filename) {
  FILE *morpheusStream = fopen(filename.c_str(), "r");

  if (!morpheusStream) return -1;

  format_id entry;
  do {
    // TODO: No fancy checks here yet.
    if (fscanf(morpheusStream, "%d", &entry.rank) != 1) {
      break;
    }
    fscanf(morpheusStream, "%d", &entry.mg_level);
    fscanf(morpheusStream, "%d", &entry.format);

    input_file.push_back(entry);
  } while (SkipUntilEol(morpheusStream) != EOF);

  fclose(morpheusStream);

  return 0;
}

#endif  // HPCG_WITH_MULTI_FORMATS
#endif  // HPCG_WITH_MULTI_FORMATS