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

#ifdef HPCG_NO_LONG_LONG
MPI_Datatype MPI_GLOBAL_INT = MPI_INT;
#else
MPI_Datatype MPI_GLOBAL_INT = MPI_LONG_LONG;
#endif  // HPCG_NO_LONG_LONG

// Construct the equivalent MPI types
MPI_Datatype MPI_FORMAT_ID, MPI_FORMAT_REPORT;

void MPI_FORMAT_ID_type_construct() {
  int lengths[3] = {1, 1, 1};
  MPI_Aint displacements[3];
  MPI_Aint base_address;
  format_id dummy_format_id;

  MPI_Get_address(&dummy_format_id, &base_address);
  MPI_Get_address(&dummy_format_id.rank, &displacements[0]);
  MPI_Get_address(&dummy_format_id.mg_level, &displacements[1]);
  MPI_Get_address(&dummy_format_id.format, &displacements[2]);
  displacements[0] = MPI_Aint_diff(displacements[0], base_address);
  displacements[1] = MPI_Aint_diff(displacements[1], base_address);
  displacements[2] = MPI_Aint_diff(displacements[2], base_address);

  MPI_Datatype types[3] = {MPI_GLOBAL_INT, MPI_INT, MPI_INT};
  MPI_Type_create_struct(3, lengths, displacements, types, &MPI_FORMAT_ID);
  MPI_Type_commit(&MPI_FORMAT_ID);
}

void MPI_FORMAT_REPORT_type_construct() {
  int lengths[5] = {1, 1, 1, 1, 1};
  MPI_Aint displacements[5];
  MPI_Aint base_address;
  format_report dummy_report;

  MPI_Get_address(&dummy_report, &base_address);
  MPI_Get_address(&dummy_report.id, &displacements[0]);
  MPI_Get_address(&dummy_report.nrows, &displacements[1]);
  MPI_Get_address(&dummy_report.ncols, &displacements[2]);
  MPI_Get_address(&dummy_report.nnnz, &displacements[3]);
  MPI_Get_address(&dummy_report.memory, &displacements[4]);
  displacements[0] = MPI_Aint_diff(displacements[0], base_address);
  displacements[1] = MPI_Aint_diff(displacements[1], base_address);
  displacements[2] = MPI_Aint_diff(displacements[2], base_address);
  displacements[3] = MPI_Aint_diff(displacements[3], base_address);
  displacements[4] = MPI_Aint_diff(displacements[4], base_address);

  // if (nullptr == MPI_FORMAT_ID) {
  MPI_FORMAT_ID_type_construct();
  // }

  MPI_Datatype types[5] = {MPI_FORMAT_ID, MPI_INT, MPI_INT, MPI_GLOBAL_INT,
                           MPI_DOUBLE};
  MPI_Type_create_struct(5, lengths, displacements, types, &MPI_FORMAT_REPORT);
  MPI_Type_commit(&MPI_FORMAT_REPORT);
}

#endif  // HPCG_NO_MPI

int count_nprocs() {
  int size = 1;
#ifndef HPCG_NO_MPI
  MPI_Comm_size(MPI_COMM_WORLD, &size);
#endif
  return size;
}

int count_nlevels() { return input_file.size() / count_nprocs(); }

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

void ReportResults() {
  std::string eol = "\n", del = "\t";
  std::string result = "";
  int rank           = 0;

#ifndef HPCG_NO_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_FORMAT_REPORT_type_construct();

  MPI_Gather(sub_report.data(), sub_report.size(), MPI_FORMAT_REPORT,
             morpheus_report.data(), sub_report.size(), MPI_FORMAT_REPORT, 0,
             MPI_COMM_WORLD);

#else
  morpheus_report.assign(sub_report.begin(), sub_report.end())
#endif

  if (rank == 0) {
    std::stringstream header;
    header << std::setw(10) << "Process" << del;
    header << std::setw(10) << "MG Level" << del;
    header << std::setw(10) << "Format" << del;
    header << std::setw(10) << "NRows" << del;
    header << std::setw(10) << "Ncols" << del;
    header << std::setw(10) << "Nnnz" << del;
    header << std::setw(10) << "Memory(Bytes)" << del;
    result += header.str() + eol;
    for (auto i = 0; i < morpheus_report.size(); i++) {
      std::stringstream val;
      val << std::setw(10) << morpheus_report[i].id.rank << del;
      val << std::setw(10) << morpheus_report[i].id.mg_level << del;
      val << std::setw(10) << morpheus_report[i].id.format << del;
      val << std::setw(10) << morpheus_report[i].nrows << del;
      val << std::setw(10) << morpheus_report[i].ncols << del;
      val << std::setw(10) << morpheus_report[i].nnnz << del;

      val << std::setprecision(14) << morpheus_report[i].memory;

      result += val.str() + eol;
    }

    std::ofstream out("morpheus-output.txt");
    out << result;
  }
}
#endif  // HPCG_WITH_MULTI_FORMATS
#endif  // HPCG_WITH_MORPHEUS