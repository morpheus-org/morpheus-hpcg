<!--
 README.md
 
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

# Morpheus-HPCG Benchmark
*Morpheus-HPCG* is a benchmark based on the [HPCG](https://github.com/hpcg-benchmark/hpcg/tree/HPCG-release-3-1-0) benchmark application, implemented using [Morpheus](https://github.com/morpheus-org/morpheus) library, allowing for heterogeneous support across a variety of Sparse Matrix Storage Formats.

## Requirements
* Git
* CMake (3.10 or later)
* MPI
* [Morpheus](https://github.com/morpheus-org/morpheus)

## Quickstart Morpheus-HPCG build
You can build Morpheus-HPCG with MPI and serial backend and dynamic matrix support as follows:
```sh
# Setup the environment with the necessary paths
CXX_COMPILER=/path/to/c++/compiler
C_COMPILER=/path/to/c/compiler
MORPHEUS_INSTALL_DIR=/path/to/morpheus/installation
MPI_DIR=/path/to/mpi/installation
# Clone morpheus-hpcg repository using git
git clone https://github.com/morpheus-org/morpheus-hpcg.git
# Go to morpheus-hpcg directory
cd morpheus-hpcg
# Create and switch to the build directory
mkdir build-MPI-Morpheus-serial && cd build-MPI-Morpheus-serial
# Configure using CMake
cmake ..  -DCMAKE_CXX_COMPILER=$CXX_COMPILER \
          -DCMAKE_CXX_EXTENSIONS=Off \
          -DHPCG_ENABLE_MORPHEUS=ON \
          -DHPCG_ENABLE_KOKKOS_SERIAL=ON \
          -DHPCG_ENABLE_MORPHEUS_DYNAMIC=ON \
          -DHPCG_MPI_DIR=$MPI_DIR \
          -DMorpheus_ROOT=$MORPHEUS_INSTALL_DIR
# build using make
make
```

## Running Morpheus HPCG benchmark application
You can run the Morpheus HPCG benchmark application by either using command line parameters or the `hpcg.dat` input file
```
morpheus-hpcg <nx> <ny> <nz> <runtime>
# where
# nx      - is the global problem size in x dimension
# ny      - is the global problem size in y dimension
# nz      - is the global problem size in z dimension
# runtime - is the desired benchmarking time in seconds (> 1800s for official runs)
```

Similarly, these parameters can be entered into an input file `hpcg.dat` in the working directory, e.g. `nx = ny = nz = 280` and `runtime = 1860`.
```
HPCG benchmark input file
Sandia National Laboratories; University of Tennessee, Knoxville
280 280 280
1860
```

## Support
Please use [the issue tracker](https://github.com/morpheus-org/morpheus-hpcg/issues) for bugs and feature requests.

## License
The [license file](https://github.com/morpheus-org/morpheus-hpcg) can be found in the main repository.