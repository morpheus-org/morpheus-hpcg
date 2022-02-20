#!/bin/sh
# build.sh
# 
# EPCC, The University of Edinburgh
# 
# (c) 2021 The University of Edinburgh
# 
# Contributing Authors:
# Christodoulos Stylianou (c.stylianou@ed.ac.uk)
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# 	http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# example command-line instruction:
# ./build.sh cirrus gnu-10.2

MACHINE="$1"
COMPILER="$2"

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "Invalid arguments.. Exiting.."
    exit -1
fi

echo "Machine::     $MACHINE"
echo "Compiler::    $COMPILER"

MODELS=("Kokkos" "Morpheus" "HPCG" "Concrete")

if [ "$MACHINE" == "archer" ]; then
    # Setup the job environment (this module needs to be loaded before any other modules)
    module load epcc-job-env
    ROOT_PATH="/work/e609/e609/cstyl"
    # TODO: Assign compiler based on Modules available
    CXX_COMPILER="/opt/gcc/10.2.0/bin/g++"
    TARGETS=("Serial" "OpenMP")
elif [ "$MACHINE" == "cirrus" ]; then
    ROOT_PATH="/lustre/home/e609/cstyl"
    if [ "$COMPILER" == "cuda-11.2" ]; then        
        CXX_COMPILER="$ROOT_PATH/kokkos/bin/nvcc_wrapper"
	    TARGETS=("Cuda")
    elif [ "$COMPILER" == "gnu-10.2" ]; then
        module unload nvidia
        module unload gcc
        module load gcc/10.2.0
        TARGETS=("Serial" "OpenMP")
        CXX_COMPILER=$(which g++)
    else
        echo "Invalid compiler argument ($COMPILER).. Exiting.."
        exit -1
    fi
    module load cmake
fi

HPCG_PATH="$ROOT_PATH/hpcg"
MORPHEUS_INSTALL_DIR="$ROOT_PATH/libs/morpheus/$COMPILER-release/"

for model in "${MODELS[@]}"
do
    for target in "${TARGETS[@]}"
    do  
        ENABLE_OPENMP="Off"
        if [ "$target" == "OpenMP" ];then
                ENABLE_OPENMP="On"
        fi
        
        if [ "$model" == "HPCG" ];then
            if [ "$target" == "Cuda" ];then
                break
            fi
            MORPHEUS_CONFIG=""
        else
            TARGET=$(echo $target | tr [a-z] [A-Z])
            MODEL=$(echo $model | tr [a-z] [A-Z])

            if [ "$MODEL" == "CONCRETE" ];then
                MODEL="KOKKOS"
                morpheus_dynamic="-DHPCG_ENABLE_MORPHEUS_DYNAMIC=Off"
            else
                morpheus_dynamic="-DHPCG_ENABLE_MORPHEUS_DYNAMIC=On"
            fi
            MORPHEUS_CONFIG="-DMorpheus_ROOT=${MORPHEUS_INSTALL_DIR} -DHPCG_ENABLE_MORPHEUS=On \
                             ${morpheus_dynamic} -DHPCG_ENABLE_${MODEL}_${TARGET}=On"
        fi

        BUILD_DIR="$HPCG_PATH/build-$COMPILER-release-$model-$target"

        mkdir -p $BUILD_DIR && cd  $BUILD_DIR
        
        CONFIG_CMD="cmake .. -DCMAKE_CXX_COMPILER=${CXX_COMPILER} -DCMAKE_CXX_EXTENSIONS=Off \
                             -DCMAKE_BUILD_TYPE=Release \
                             -DHPCG_ENABLE_DETAILED_DEBUG=Off -DHPCG_ENABLE_OPENMP=${ENABLE_OPENMP} \
                             -DHPCG_ENABLE_LONG_LONG=Off $MORPHEUS_CONFIG"

        if [ "$target" == "Cuda" ]
        then    
            tempout="$BUILD_DIR/out-$model-$target.sh"
            if [ "$COMPILER" == "cuda-11.2" ]; then
                echo "#!/bin/sh
                      module unload nvidia
                      module unload gcc
                      module load nvidia/cuda-11.2
                      module swap gcc/6.3.0 gcc/8.2.0
		              module load cmake
                      $CONFIG_CMD && make -j" > $tempout
            fi

            ACCOUNT="e609"
            RESOURCES="--time=00:05:00 --exclusive --nodes=1 --cpus-per-task=40"
            SYSTEM="--gres=gpu:4 --partition=gpu-cascade --qos=gpu"
            SCHEDULER_ARGS="--account=$ACCOUNT --job-name=hpcg_$model $RESOURCES $SYSTEM"
            SCHEDULER_LAUNCER="sbatch"
            
            $SCHEDULER_LAUNCER $SCHEDULER_ARGS $tempout
        else
            $CONFIG_CMD && make -j
        fi
    done
done
