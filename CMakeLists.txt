# Modifications (c) 2022 The University of Edinburgh
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#
# HPCG Benchmark CMake configuration
#
cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(
  morpheus-hpcg
  VERSION 3.1.1
  LANGUAGES CXX
  DESCRIPTION
    "High Performance Conjugate Gradient Benchmark (HPCG) using Morpheus")

macro(APPEND_GLOB VAR)
  file(GLOB LOCAL_TMP_VAR ${ARGN})
  list(APPEND ${VAR} ${LOCAL_TMP_VAR})
endmacro()

#
# Options
#
option(HPCG_ENABLE_CONTIGUOUS_ARRAYS
       "Enable contiguous arrays for better cache pre-fetch" OFF)
option(HPCG_ENABLE_CUBIC_RADICAL_SEARCH
       "Enable faster search for optimal 3D process grid" OFF)
option(HPCG_ENABLE_DEBUG "Enable debug build" OFF)
option(HPCG_ENABLE_DETAILED_DEBUG "Enable detailed debug build" OFF)
option(HPCG_ENABLE_MPI "Enable MPI support" OFF)
option(HPCG_ENABLE_LONG_LONG
       "Enable use of 'long long' type for global indices" ON)
option(HPCG_ENABLE_OPENMP "Enable OpenMP support" OFF)
option(HPCG_ENABLE_MG "Enable MG Preconditioner in timing runs." ON)
# Build with Morpheus
option(HPCG_ENABLE_MORPHEUS "Enable Morpheus library" OFF)
if(HPCG_ENABLE_MORPHEUS)
  option(HPCG_ENABLE_MORPHEUS_DYNAMIC
         "Enable Morpheus library with dynamic matrix support" OFF)
  option(HPCG_ENABLE_KOKKOS_SERIAL "Enable Morpheus Serial Execution Space" OFF)
  option(HPCG_ENABLE_KOKKOS_OPENMP "Enable Morpheus OpenMP Execution Space" OFF)
  option(HPCG_ENABLE_KOKKOS_CUDA "Enable Morpheus Cuda Execution Space" OFF)
  option(HPCG_ENABLE_KOKKOS_HIP "Enable Morpheus HIP Execution Space" OFF)
  option(
    HPCG_ENABLE_MULTI_FORMATS
    "Enabling multiple formats across processes and MG levels using the input file."
    OFF)
  option(
    HPCG_ENABLE_SPLIT_DISTRIBUTED
    "Enabling split between on-process and ghost elements of the local matrix."
    OFF)
endif()

set(HPCG_SOURCES)
append_glob(HPCG_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
if(HPCG_ENABLE_MORPHEUS)
  append_glob(HPCG_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/morpheus/*.cpp)
endif()

add_executable(morpheus-hpcg ${HPCG_SOURCES})
target_include_directories(morpheus-hpcg
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(HPCG_ENABLE_CONTIGUOUS_ARRAYS)
  target_compile_definitions(morpheus-hpcg PRIVATE HPCG_CONTIGUOUS_ARRAYS)
endif()

if(HPCG_ENABLE_CUBIC_RADICAL_SEARCH)
  target_compile_definitions(morpheus-hpcg PRIVATE HPCG_CUBIC_RADICAL_SEARCH)
endif()

if(HPCG_ENABLE_DEBUG)
  target_compile_definitions(morpheus-hpcg PRIVATE HPCG_DEBUG)
endif()

if(HPCG_ENABLE_DETAILED_DEBUG)
  target_compile_definitions(morpheus-hpcg PRIVATE HPCG_DETAILED_DEBUG)
endif()

if(HPCG_ENABLE_MPI)
  set(MPI_CXX_SKIP_MPICXX ON)
  find_package(MPI REQUIRED)
  target_link_libraries(morpheus-hpcg ${MPI_CXX_LIBRARIES})
else()
  target_compile_definitions(morpheus-hpcg PRIVATE HPCG_NO_MPI)
endif()

if(NOT HPCG_ENABLE_LONG_LONG)
  target_compile_definitions(morpheus-hpcg PRIVATE HPCG_NO_LONG_LONG)
endif()

if(HPCG_ENABLE_OPENMP)
  find_package(OpenMP REQUIRED)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  target_link_libraries(morpheus-hpcg ${OpenMP_CXX_LIBRARIES})
else()
  target_compile_definitions(morpheus-hpcg PRIVATE HPCG_NO_OPENMP)
endif()

# Disabling it will result in disabling MG in timed run
if(HPCG_ENABLE_MG)
  target_compile_definitions(morpheus-hpcg PRIVATE HPCG_WITH_MG)
  message(STATUS "Preconditioner: MG (ON)")
endif()

if(HPCG_ENABLE_MORPHEUS)
  if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
    message(STATUS "Setting policy CMP0074 to use <Package>_ROOT variables")
    cmake_policy(SET CMP0074 NEW)
  endif()

  message(STATUS "HPCG is build with Morpheus")

  find_package(Morpheus REQUIRED)
  target_link_libraries(morpheus-hpcg Morpheus::morpheus)
  target_compile_definitions(morpheus-hpcg PRIVATE HPCG_WITH_MORPHEUS)
  if(HPCG_ENABLE_MORPHEUS_DYNAMIC)
    target_compile_definitions(morpheus-hpcg PRIVATE HPCG_WITH_MORPHEUS_DYNAMIC)
    message(STATUS "Morpheus Dynamic Matrix support is enabled.")
  endif()

  if(HPCG_ENABLE_KOKKOS_SERIAL)
    target_compile_definitions(morpheus-hpcg PRIVATE HPCG_WITH_KOKKOS_SERIAL)
    message(STATUS "Execution Space: Kokkos::Serial")
  elseif(HPCG_ENABLE_KOKKOS_OPENMP)
    target_compile_definitions(morpheus-hpcg PRIVATE HPCG_WITH_KOKKOS_OPENMP)
    message(STATUS "Execution Space: Kokkos::OpenMP")
  elseif(HPCG_ENABLE_KOKKOS_CUDA)
    target_compile_definitions(morpheus-hpcg PRIVATE HPCG_WITH_KOKKOS_CUDA)
    message(STATUS "Execution Space: Kokkos::Cuda")
  elseif(HPCG_ENABLE_KOKKOS_HIP)
    target_compile_definitions(morpheus-hpcg PRIVATE HPCG_WITH_KOKKOS_HIP)
    message(STATUS "Execution Space: Kokkos::Experimental::HIP")
  else()
    message(
      FATAL_ERROR "Please enable an execution space to be used with HPCG.")
  endif()

  if(HPCG_ENABLE_MULTI_FORMATS)
    target_compile_definitions(morpheus-hpcg PRIVATE HPCG_WITH_MULTI_FORMATS)
    message(STATUS "Multiple Formats: ON")
  endif()

  if(HPCG_ENABLE_SPLIT_DISTRIBUTED)
    target_compile_definitions(morpheus-hpcg
                               PRIVATE HPCG_WITH_SPLIT_DISTRIBUTED)
    message(STATUS "Split distributed data structures: ON")
  endif()
endif()

# target_compile_options(morpheus-hpcg PRIVATE -O3)
