/**
 * Morpheus_Parser.hpp
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

#include "morpheus/Morpheus_Parser.hpp"

#ifdef HPCG_WITH_MORPHEUS
#include "morpheus/Morpheus.hpp"
#if defined(HPCG_WITH_MULTI_FORMATS)
#include "morpheus/Morpheus_ReadHpcgDat.hpp"
#endif  // HPCG_WITH_MULTI_FORMATS

#ifndef HPCG_NO_MPI
#include <mpi.h>
#endif

#include <string>

int local_matrix_fmt;
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
int ghost_matrix_fmt;
#endif  // HPCG_WITH_SPLIT_DISTRIBUTED

#ifdef HPCG_WITH_MULTI_FORMATS
std::vector<format_id> local_input_file;
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
std::vector<format_id> ghost_input_file;
#endif  // HPCG_WITH_SPLIT_DISTRIBUTED
#endif  // HPCG_WITH_MULTI_FORMATS

static int startswith(const char* s, const char* prefix) {
  size_t n = strlen(prefix);
  if (strncmp(s, prefix, n)) return 0;
  return 1;
}

#if defined(HPCG_WITH_MULTI_FORMATS)
void ParseInputFileFormats_Impl(int argc, char* argv[], std::string prefix,
                                std::vector<format_id>& input_file) {
  std::string filename, tag("--hpcg-" + prefix + "-formats=");

  int rank = 0;
#ifndef HPCG_NO_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

  for (int i = 1; i <= argc && argv[i]; ++i) {
    if (startswith(argv[i], tag.c_str())) {
      filename = std::string(argv[i]);
      filename.erase(filename.find(tag), tag.length());

      if (rank == 0) {
        std::cout << "Reading HPCG input file for " << prefix
                  << " formats from: " << filename << std::endl;
      }

      ReadMorpheusDat(filename, input_file);
    }
  }
}

void ParseInputFileFormats(int argc, char* argv[]) {
  ParseInputFileFormats_Impl(argc, argv, "local", local_input_file);

#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
  ParseInputFileFormats_Impl(argc, argv, "ghost", ghost_input_file);
#endif  // HPCG_WITH_SPLIT_DISTRIBUTED}
}
#endif  // HPCG_WITH_MULTI_FORMATS

void ParseFormat_Impl(int argc, char* argv[], std::string prefix,
                      int* matrix_fmt) {
  std::string entry, tag("--" + prefix + "-format=");
  *matrix_fmt = Morpheus::CSR_FORMAT;  // Default is CSR
  for (int i = 1; i <= argc && argv[i]; ++i) {
    if (startswith(argv[i], tag.c_str())) {
      entry = std::string(argv[i]);
      entry.erase(entry.find(tag), tag.length());

      *matrix_fmt = std::stoi(entry);
    }
  }
}

void ParseFormats(int argc, char* argv[]) {
  ParseFormat_Impl(argc, argv, "local", &local_matrix_fmt);

#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
  ParseFormat_Impl(argc, argv, "ghost", &ghost_matrix_fmt);
#endif  // HPCG_WITH_SPLIT_DISTRIBUTED
}

#endif  // HPCG_WITH_MORPHEUS