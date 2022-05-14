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

#include "morpheus/Morpheus_ReportResults.hpp"

#if defined(HPCG_WITH_MORPHEUS)
#if defined(HPCG_WITH_MULTI_FORMATS)
#include "morpheus/Morpheus.hpp"
#include "morpheus/Morpheus_Timer.hpp"

#include <sstream>

#ifndef HPCG_NO_MPI
#include <mpi.h>
#endif

int count_nprocs() {
  int size = 1;
#ifndef HPCG_NO_MPI
  MPI_Comm_size(MPI_COMM_WORLD, &size);
#endif
  return size;
}

int count_nlevels() { return fmt_tuple.nentries / count_nprocs(); }

void ReportTimingResults() {
  std::string eol = "\n", del = "\t";
  std::string result = "";
  std::vector<std::string> timers(
      {"SPMV ", "SYMGS", "MG   ", "Halo ", "CG   "});

  int id;
  int nprocs  = count_nprocs();
  int nlevels = count_nlevels();
  int rank    = 0;

#ifndef HPCG_NO_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Gather(sub_mtimers.data(), sub_mtimers.size(), MPI_DOUBLE, mtimers.data(),
             sub_mtimers.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);

#else
  mtimers.assign(sub_mtimers.begin(), sub_mtimers.end())
#endif

  if (rank == 0) {
#ifdef HPCG_WITH_MG
    auto mg_levels = nlevels;
#else
    auto mg_levels = 1;
#endif
    for (auto pid = 0; pid < nprocs; pid++) {
      for (auto lid = 0; lid < mg_levels; lid++) {
        for (auto tid = 0; tid < ntimers; tid++) {
          id = pid * nlevels * ntimers + lid * ntimers + tid;
          std::stringstream val;
          val << std::setprecision(14) << mtimers[id];
          result += std::to_string(pid) + del + std::to_string(lid) + del +
                    timers[tid] + del + val.str() + eol;
        }
      }
    }

    std::ofstream out("morpheus-timings-output.txt");
    out << result;
  }
}

// void ReportResults() {
//   std::string eol = "\n", del = "\t";
//   std::string result = "";
//   std::vector<std::string> timers(
//       {"SPMV ", "SYMGS", "MG   ", "Halo ", "CG   "});

//   int id;
//   int nprocs  = count_nprocs();
//   int nlevels = count_nlevels();
//   int rank    = 0;

// #ifndef HPCG_NO_MPI
//   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//   MPI_Gather(sub_mtimers.data(), sub_mtimers.size(), MPI_DOUBLE,
//   mtimers.data(),
//              sub_mtimers.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);

// #else
//   mtimers.assign(sub_mtimers.begin(), sub_mtimers.end())
// #endif

//   if (rank == 0) {
// #ifdef HPCG_WITH_MG
//     auto mg_levels = nlevels;
// #else
//     auto mg_levels = 1;
// #endif
//     for (auto pid = 0; pid < nprocs; pid++) {
//       for (auto lid = 0; lid < mg_levels; lid++) {
//         for (auto tid = 0; tid < ntimers; tid++) {
//           id = pid * nlevels * ntimers + lid * ntimers + tid;
//           std::stringstream val;
//           val << std::setprecision(14) << mtimers[id];
//           result += std::to_string(pid) + del + std::to_string(lid) + del +
//                     timers[tid] + del + val.str() + eol;
//         }
//       }
//     }

//     std::ofstream out("morpheus-timings-output.txt");
//     out << result;
//   }
// }
#endif  // HPCG_WITH_MULTI_FORMATS
#endif  // HPCG_WITH_MORPHEUS