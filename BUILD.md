<!--
 BUILD.md
 
 EPCC, The University of Edinburgh
 
 (c) 2022 The University of Edinburgh
 
 Contributing Authors:
 Christodoulos Stylianou (c.stylianou@ed.ac.uk)
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 	http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
-->

# Morpheus-HPCG Build
## Setting up the enivronment
```sh
$ CXX_COMPILER=/path/to/c++/compiler
$ C_COMPILER=/path/to/c/compiler
$ MORPHEUS_INSTALL_DIR=/path/to/morpheus/installation
$ MPI_DIR=/path/to/mpi/installation
```

### Reference Build
```sh
$ mkdir build-MPI-reference && cd build-MPI-reference
$ cmake ..  -DCMAKE_CXX_COMPILER=$CXX_COMPILER \
            -DCMAKE_CXX_EXTENSIONS=Off \
            -DHPCG_MPI_DIR=$MPI_DIR 
$ make
```

### HPCG+Morpheus backend (Serial) Build
```sh
$ mkdir build-MPI-Morpheus-serial && cd build-MPI-Morpheus-serial
$ cmake ..  -DCMAKE_CXX_COMPILER=$CXX_COMPILER \
            -DCMAKE_CXX_EXTENSIONS=Off \
            -DHPCG_ENABLE_MORPHEUS=ON \
            -DHPCG_ENABLE_KOKKOS_SERIAL=ON \
            -DHPCG_MPI_DIR=$MPI_DIR \
            -DMorpheus_ROOT=$MORPHEUS_INSTALL_DIR
$ make
```

## CMake Option Listing
### HPCG Options
* HPCG_ENABLE_CONTIGUOUS_ARRAYS: BOOL
  * Whether to enable contiguous arrays for better cache pre-fetch.
  * Default: OFF
* HPCG_ENABLE_CUBIC_RADICAL_SEARCH: BOOL
  * Whether to enable faster search for optimal 3D process grid.
  * Default: OFF
* HPCG_ENABLE_DEBUG: BOOL
  * Whether to enable debug build.
  * Default: OFF.
* HPCG_ENABLE_DETAILED_DEBUG: BOOL
  * Whether to enable detailed debug build.
  * Default: OFF.
* HPCG_ENABLE_MPI : BOOL
  * Whether to enable MPI support.
  * Default: On.
* HPCG_ENABLE_LONG_LONG : BOOL
  * Whether to enable use of 'long long' type for global indices.
  * Default: On.
* HPCG_ENABLE_OPENMP: BOOL
  * Whether to enable OPENMP support.
  * Default: Off.

### Morpheus-HPCG Options
* HPCG_ENABLE_MORPHEUS: BOOL
  * Whether to enable Morpheus Library.
  * Default: OFF
* HPCG_ENABLE_MORPHEUS_DYNAMIC: BOOL
  * Whether to enable Morpheus Library with dynamic matrix support.
  * Default: OFF
* HPCG_ENABLE_MORPHEUS_SERIAL: BOOL
  * Whether to enable Morpheus Serial Execution Space.
  * Default: OFF
* HPCG_ENABLE_MORPHEUS_OPENMP: BOOL
  * Whether to enable Morpheus OpenMP Execution Space.
  * Default: OFF
* HPCG_ENABLE_MORPHEUS_CUDA: BOOL
  * Whether to enable Morpheus Cuda Execution Space.
  * Default: OFF
