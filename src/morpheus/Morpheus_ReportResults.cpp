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
#include "morpheus/Morpheus.hpp"

#if defined(HPCG_WITH_MULTI_FORMATS)
#include "morpheus/Morpheus_Timer.hpp"
#endif

#include <sstream>
#include <fstream>

#ifndef HPCG_NO_MPI
#include <mpi.h>

#ifdef HPCG_NO_LONG_LONG
MPI_Datatype MPI_GLOBAL_INT = MPI_INT;
#else
MPI_Datatype MPI_GLOBAL_INT = MPI_LONG_LONG;
#endif  // HPCG_NO_LONG_LONG

// Construct the equivalent MPI types
MPI_Datatype MPI_FORMAT_ID, MPI_FORMAT_REPORT, MPI_MORPHEUS_TIMERS;

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

  MPI_FORMAT_ID_type_construct();

  MPI_Datatype types[5] = {MPI_FORMAT_ID, MPI_INT, MPI_INT, MPI_GLOBAL_INT,
                           MPI_DOUBLE};
  MPI_Type_create_struct(5, lengths, displacements, types, &MPI_FORMAT_REPORT);
  MPI_Type_commit(&MPI_FORMAT_REPORT);
}
#if defined(HPCG_WITH_MULTI_FORMATS)
void MPI_MORPHEUS_TIMERS_type_construct() {
  int lengths[7] = {1, 1, 1, 1, 1, 1, 1};
  MPI_Aint displacements[7];
  MPI_Aint base_address;
  morpheus_timers dummy_timer;

  MPI_Get_address(&dummy_timer, &base_address);
  MPI_Get_address(&dummy_timer.SPMV, &displacements[0]);
  MPI_Get_address(&dummy_timer.SYMGS, &displacements[1]);
  MPI_Get_address(&dummy_timer.MG, &displacements[2]);
  MPI_Get_address(&dummy_timer.HALO_SWAP, &displacements[3]);
  MPI_Get_address(&dummy_timer.CG, &displacements[4]);
  MPI_Get_address(&dummy_timer.SPMV_LOCAL, &displacements[5]);
  MPI_Get_address(&dummy_timer.SPMV_GHOST, &displacements[6]);
  displacements[0] = MPI_Aint_diff(displacements[0], base_address);
  displacements[1] = MPI_Aint_diff(displacements[1], base_address);
  displacements[2] = MPI_Aint_diff(displacements[2], base_address);
  displacements[3] = MPI_Aint_diff(displacements[3], base_address);
  displacements[4] = MPI_Aint_diff(displacements[4], base_address);
  displacements[5] = MPI_Aint_diff(displacements[5], base_address);
  displacements[6] = MPI_Aint_diff(displacements[6], base_address);

  MPI_Datatype types[7] = {MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE,
                           MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE};
  MPI_Type_create_struct(7, lengths, displacements, types,
                         &MPI_MORPHEUS_TIMERS);
  MPI_Type_commit(&MPI_MORPHEUS_TIMERS);
}
#endif  // HPCG_WITH_MULTI_FORMATS

#endif  // HPCG_NO_MPI

#if defined(HPCG_WITH_MULTI_FORMATS)
void ReportTimingResults() {
  std::string eol = "\n", del = ",";
  std::string result = "";

  int rank = 0;
#ifndef HPCG_NO_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_MORPHEUS_TIMERS_type_construct();

  MPI_Gather(sub_mtimers.data(), sub_mtimers.size(), MPI_MORPHEUS_TIMERS,
             mtimers.data(), sub_mtimers.size(), MPI_MORPHEUS_TIMERS, 0,
             MPI_COMM_WORLD);
#else
  mtimers.assign(sub_mtimers.begin(), sub_mtimers.end())
#endif

  if (rank == 0) {
#ifdef HPCG_WITH_MG
    size_t report_mg_levels = sub_mtimers.size();
#else
    size_t report_mg_levels = 1;
#endif

    std::stringstream header;
    header << "Process" << del << "MG_Level" << del << "SPMV" << del
           << "SPMV_Local" << del
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
           << "SPMV_Ghost" << del
#endif
           << "SYMGS" << del << "MG" << del << "Halo_Swap" << del << "CG";
    result += header.str() + eol;

    for (size_t i = 0; i < mtimers.size(); i++) {
      size_t current_proc  = i / sub_mtimers.size();
      size_t current_level = i % sub_mtimers.size();

      if (current_level >= report_mg_levels) continue;

      std::stringstream val;
      val << current_proc << del << current_level << del << std::fixed
          << std::setprecision(14) << mtimers[i].SPMV << del << std::fixed
          << std::setprecision(14) << mtimers[i].SPMV_LOCAL << del
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
          << std::fixed << std::setprecision(14) << mtimers[i].SPMV_GHOST << del
#endif
          << std::fixed << std::setprecision(14) << mtimers[i].SYMGS << del
          << std::fixed << std::setprecision(14) << mtimers[i].MG << del
          << std::fixed << std::setprecision(14) << mtimers[i].HALO_SWAP << del
          << std::fixed << std::setprecision(14) << mtimers[i].CG;

      result += val.str() + eol;
    }

    std::ofstream out("morpheus-timings-output.txt");
    out << result;
  }
}
#endif  // HPCG_WITH_MULTI_FORMATS

void ReportResults_Impl(std::string prefix, std::vector<format_report>& report,
                        std::vector<format_report>& sub_report) {
  std::string eol = "\n", del = ",";
  std::string result = "";
  int rank           = 0;

#ifndef HPCG_NO_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_FORMAT_REPORT_type_construct();

  MPI_Gather(sub_report.data(), sub_report.size(), MPI_FORMAT_REPORT,
             report.data(), sub_report.size(), MPI_FORMAT_REPORT, 0,
             MPI_COMM_WORLD);
#else
  report.assign(sub_report.begin(), sub_report.end());
#endif

  if (rank == 0) {
    std::stringstream header;
    header << "Process" << del << "MG_Level" << del << "Format" << del
           << "NRows" << del << "Ncols" << del << "Nnnz" << del
           << "Memory(Bytes)";

    result += header.str() + eol;
    for (size_t i = 0; i < report.size(); i++) {
      std::stringstream val;
      val << report[i].id.rank << del << report[i].id.mg_level << del
          << report[i].id.format << del << report[i].nrows << del
          << report[i].ncols << del << report[i].nnnz << del
          << std::setprecision(14) << report[i].memory;

      result += val.str() + eol;
    }

    std::ofstream out("morpheus-" + prefix + "-output.txt");
    out << result;
  }
}

void ReportResults() {
  ReportResults_Impl("local", local_morpheus_report, local_sub_report);
#if defined(HPCG_WITH_SPLIT_DISTRIBUTED)
  ReportResults_Impl("ghost", ghost_morpheus_report, ghost_sub_report);
#endif  // HPCG_WITH_SPLIT_DISTRIBUTED
}
#endif  // HPCG_WITH_MORPHEUS