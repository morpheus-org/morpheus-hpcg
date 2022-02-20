#!/bin/sh
# process.sh
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
# ./process.sh cirrus gnu-10.2 Kokkos OpenMP hpcg-bench timings

MACHINE="$1"
COMPILER="$2"
MODEL="$3" 
TARGET="$4"
EXPERIMENT="$5"
FILENAME="$6"

if [ "$#" -lt 6 ]; then
    echo "Warning! Only $# out if 6 were provided."
    echo "Arguments Provided are: $1 $2 $3 $4 $5 $6"

    echo "Defaulted arguments:"
    if [ -z "$1" ]; then
        MACHINE="cirrus"
        echo -e "\tMachine::        $MACHINE"
    fi

    if [ -z "$2" ]; then
        COMPILER="gnu-10.2"
        echo -e "\tCompiler::       $COMPILER"
    fi

    if [ -z "$3" ]; then
        MODEL="HPCG"
        echo -e "\tModel::          $MODEL"
    fi

    if [ -z "$4" ]; then
        TARGET="OpenMP"
        echo -e "\tTarget::         $TARGET"
    fi

    if [ -z "$5" ]; then
        EXPERIMENT="hpcg-bench"
        echo -e "\tExperiment::     $EXPERIMENT"
    fi

    if [ -z "$6" ]; then
        FILENAME="timings"
        echo -e "\tFilename::       performance-$FILENAME-$MACHINE.csv"
    fi
fi

echo -e "\nParsed Runtime Parameters:"
echo -e "=========================="
echo -e "Machine::        $MACHINE"
echo -e "Compiler::       $COMPILER"
echo -e "Model::          $MODEL"
echo -e "Target::         $TARGET"
echo -e "Experiment::     $EXPERIMENT"
echo -e "Filename::       $FILENAME"


if [ "$MACHINE" == "archer" ]; then
    ROOT_PATH="/work/e609/e609/cstyl"
elif [ "$MACHINE" == "cirrus" ]; then
    ROOT_PATH="/lustre/home/e609/cstyl"
fi
HPCG_PATH="$ROOT_PATH/hpcg"

echo "Root Path::   $ROOT_PATH"
echo "HPCG Path::   $HPCG_PATH"

RESULTS_FILE="$HPCG_PATH/benchmarks/results/processed/$EXPERIMENT/performance-$FILENAME-$MACHINE-$COMPILER-$MODEL-$TARGET.csv"
OUTPUT_PATH="$HPCG_PATH/benchmarks/results/$EXPERIMENT/performance-$MODEL-$TARGET"

mkdir -p $(dirname $RESULTS_FILE)

# CSV Header
header="procs,threads,gnx,gny,gnz,npx,npy,npz,nx,ny,nz"
header="$header,ValidBench,Rating,ConvergenceTest,SymmetryTest,IterationCountTest,ReproducibilityTest"
header="$header,ConvergenceTestIterCount,ConvergenceTestExpIterCount,SymmetryTestSpmv"
header="$header,IterationCountTestRefCGIter,IterationCountTestOptCGIter,IterationCountTestRefIter"
header="$header,IterationCountTestOptIter,ReproducibilityTestMean,ReproducibilityTestVar"
header="$header,LinearSize,LinearNnz,Format,Rows,Columns,NonZeros"
header="$header,TimeTotal,TimeOpt,TimeDot,TimeWaxpby,TimeSpmv,TimeMG"
header="$header,FPTotal,FPDot,FPWaxpby,FPSpmv,FPMG"
header="$header,BWTotal,BWRead,BWWrite"
header="$header,AITotal,AIDot,AIWaxpby,AISpmv,AIMG"
header="$header,SetupTime,MemoryData,MemoryOpt,OptOverheadsTime,OptimizedTimes"

echo "$header"  2>&1 | tee "$RESULTS_FILE"

for PROBLEM_PATH in "$OUTPUT_PATH"/*
do
    PROBLEM=$(basename "$PROBLEM_PATH")

    if [[ -d $PROBLEM_PATH ]]; then
        for THREADS_PATH in "$PROBLEM_PATH"/*
        do
            if [[ -d $THREADS_PATH ]]; then
                THREAD=$(basename "$THREADS_PATH")
                for REPS_PATH in "$THREADS_PATH"/*
                do  
                    REP=$(basename "$REPS_PATH")
                    for FMTS_PATH in "$REPS_PATH"/*
                    do  
                        if [[ -d $FMTS_PATH ]]; then
                            FMT=$(basename "$FMTS_PATH")
                            FILE="$PROBLEM_PATH/$THREAD/$REP/$FMT/out.txt"
                            ORIG_FILE="$PROBLEM_PATH/$THREAD/$REP/$FMT/HPCG**"
                            mv $ORIG_FILE $FILE

                            procs=$(cut -d "=" -f2 <<< $(awk '/Machine Summary::Distributed/ {printf "%s",$0}' "$FILE"))
                            threads=$(cut -d "=" -f2 <<< $(awk '/Machine Summary::Threads/ {printf "%s",$0}' "$FILE"))
                            gnx=$(cut -d "=" -f2 <<< $(awk '/Global Problem Dimensions::Global nx/ {printf "%s",$0}' "$FILE"))
                            gny=$(cut -d "=" -f2 <<< $(awk '/Global Problem Dimensions::Global ny/ {printf "%s",$0}' "$FILE"))
                            gnz=$(cut -d "=" -f2 <<< $(awk '/Global Problem Dimensions::Global nz/ {printf "%s",$0}' "$FILE"))
                            npx=$(cut -d "=" -f2 <<< $(awk '/Processor Dimensions::npx/ {printf "%s",$0}' "$FILE"))
                            npy=$(cut -d "=" -f2 <<< $(awk '/Processor Dimensions::npy/ {printf "%s",$0}' "$FILE"))
                            npz=$(cut -d "=" -f2 <<< $(awk '/Processor Dimensions::npz/ {printf "%s",$0}' "$FILE"))
                            nx=$(cut -d "=" -f2 <<< $(awk '/Local Domain Dimensions::nx/ {printf "%s",$0}' "$FILE"))
                            ny=$(cut -d "=" -f2 <<< $(awk '/Local Domain Dimensions::ny/ {printf "%s",$0}' "$FILE"))
                            nz=$(cut -d "=" -f2 <<< $(awk '/Local Domain Dimensions::nz/ {printf "%s",$0}' "$FILE"))

                            ConvergenceTest=$(cut -d "=" -f2 <<< $(awk '/Spectral Convergence Tests::Result/ {printf "%s",$0}' "$FILE"))
                            SymmetryTest=$(cut -d "=" -f2 <<< $(awk '/^Departure from Symmetry.*Result/ {printf "%s",$0}' "$FILE"))
                            IterationCountTest=$(cut -d "=" -f2 <<< $(awk '/Iteration Count Information::Result/ {printf "%s",$0}' "$FILE"))
                            ReproducibilityTest=$(cut -d "=" -f2 <<< $(awk '/Reproducibility Information::Result/ {printf "%s",$0}' "$FILE"))
                            ValidBench=$(awk '/Final Summary::HPCG result/ {printf "%s",$5}' "$FILE")
                            Rating=$(cut -d "=" -f2 <<< $(awk '/Final Summary::HPCG result/ {printf "%s",$0}' "$FILE"))

                            ConvergenceTestIterCount=$(cut -d "=" -f2 <<< $(awk '/^Spectral Convergence Tests::Unpreconditioned::Maximum/ {printf "%s",$0}' "$FILE"))
                            ConvergenceTestExpIterCount=$(cut -d "=" -f2 <<< $(awk '/Spectral Convergence Tests::Unpreconditioned::Expected/ {printf "%s",$0}' "$FILE"))
                            SymmetryTestSpmv=$(cut -d "=" -f2 <<< $(awk '/^Departure from Symmetry.*Departure for SpMV/ {printf "%s",$0}' "$FILE"))
                            IterationCountTestRefCGIter=$(cut -d "=" -f2 <<< $(awk '/Iteration Count Information::Reference/ {printf "%s",$0}' "$FILE"))
                            IterationCountTestOptCGIter=$(cut -d "=" -f2 <<< $(awk '/Iteration Count Information::Optimized/ {printf "%s",$0}' "$FILE"))
                            IterationCountTestRefIter=$(cut -d "=" -f2 <<< $(awk '/^Iteration Count.*reference iterations/ {printf "%s",$0}' "$FILE"))
                            IterationCountTestOptIter=$(cut -d "=" -f2 <<< $(awk '/^Iteration Count.*optimized iterations/ {printf "%s",$0}' "$FILE"))
                            ReproducibilityTestMean=$(cut -d "=" -f2 <<< $(awk '/^Reproducibility Information.*mean/ {printf "%s",$0}' "$FILE"))
                            ReproducibilityTestVar=$(cut -d "=" -f2 <<< $(awk '/^Reproducibility Information.*variance/ {printf "%s",$0}' "$FILE"))

                            LinearSize=$(cut -d "=" -f2 <<< $(awk '/Linear System Information::Number of Equations/ {printf "%s",$0}' "$FILE"))
                            LinearNnz=$(cut -d "=" -f2 <<< $(awk '/Linear System Information::Number of Nonzero Terms/ {printf "%s",$0}' "$FILE"))
                            Format=$(cut -d "=" -f2 <<< $(awk '/Morpheus::Format/ {printf "%s",$0}' "$FILE"))
                            Rows=$(cut -d "=" -f2 <<< $(awk '/Morpheus::Rows/ {printf "%s",$0}' "$FILE"))
                            Columns=$(cut -d "=" -f2 <<< $(awk '/Morpheus::Columns/ {printf "%s",$0}' "$FILE"))
                            NonZeros=$(cut -d "=" -f2 <<< $(awk '/Morpheus::Non Zeros/ {printf "%s",$0}' "$FILE"))

                            TimeTotal=$(cut -d "=" -f2 <<< $(awk '/Benchmark Time Summary::Total/ {printf "%s",$0}' "$FILE"))
                            TimeOpt=$(cut -d "=" -f2 <<< $(awk '/Benchmark Time Summary::Optimization/ {printf "%s",$0}' "$FILE"))
                            TimeDot=$(cut -d "=" -f2 <<< $(awk '/Benchmark Time Summary::DDOT/ {printf "%s",$0}' "$FILE"))
                            TimeWaxpby=$(cut -d "=" -f2 <<< $(awk '/Benchmark Time Summary::WAXPBY/ {printf "%s",$0}' "$FILE"))
                            TimeSpmv=$(cut -d "=" -f2 <<< $(awk '/Benchmark Time Summary::SpMV/ {printf "%s",$0}' "$FILE"))
                            TimeMG=$(cut -d "=" -f2 <<< $(awk '/Benchmark Time Summary::MG/ {printf "%s",$0}' "$FILE"))

                            FPTotal=$(cut -d "=" -f2 <<< $(awk '/Floating Point Operations Summary::Total=/ {printf "%s",$0}' "$FILE"))
                            FPDot=$(cut -d "=" -f2 <<< $(awk '/Floating Point Operations Summary::Raw DDOT/ {printf "%s",$0}' "$FILE"))
                            FPWaxpby=$(cut -d "=" -f2 <<< $(awk '/Floating Point Operations Summary::Raw WAXPBY/ {printf "%s",$0}' "$FILE"))
                            FPSpmv=$(cut -d "=" -f2 <<< $(awk '/Floating Point Operations Summary::Raw SpMV/ {printf "%s",$0}' "$FILE"))
                            FPMG=$(cut -d "=" -f2 <<< $(awk '/Floating Point Operations Summary::Raw MG/ {printf "%s",$0}' "$FILE"))

                            BWTotal=$(cut -d "=" -f2 <<< $(awk '/GB\/s Summary::Raw Total/ {printf "%s",$0}' "$FILE"))
                            BWRead=$(cut -d "=" -f2 <<< $(awk '/GB\/s Summary::Raw Read/ {printf "%s",$0}' "$FILE"))
                            BWWrite=$(cut -d "=" -f2 <<< $(awk '/GB\/s Summary::Raw Write/ {printf "%s",$0}' "$FILE"))

                            AITotal=$(cut -d "=" -f2 <<< $(awk '/GFLOP\/s Summary::Raw Total=/ {printf "%s",$0}' "$FILE"))
                            AIDot=$(cut -d "=" -f2 <<< $(awk '/GFLOP\/s Summary::Raw DDOT/ {printf "%s",$0}' "$FILE"))
                            AIWaxpby=$(cut -d "=" -f2 <<< $(awk '/GFLOP\/s Summary::Raw WAXPBY/ {printf "%s",$0}' "$FILE"))
                            AISpmv=$(cut -d "=" -f2 <<< $(awk '/GFLOP\/s Summary::Raw SpMV/ {printf "%s",$0}' "$FILE"))
                            AIMG=$(cut -d "=" -f2 <<< $(awk '/GFLOP\/s Summary::Raw MG/ {printf "%s",$0}' "$FILE"))

                            SetupTime=$(cut -d "=" -f2 <<< $(awk '/Setup Information::Setup/ {printf "%s",$0}' "$FILE"))
                            MemoryData=$(cut -d "=" -f2 <<< $(awk '/Memory Use Information::Total memory used for data \(Gbytes\)/ {printf "%s",$0}' "$FILE"))
                            MemoryOpt=$(cut -d "=" -f2 <<< $(awk '/Memory Use Information::Memory used for OptimizeProblem data/ {printf "%s",$0}' "$FILE"))
                            OptOverheadsTime=$(cut -d "=" -f2 <<< $(awk '/User Optimization Overheads::Optimization phase time \(sec\)/ {printf "%s",$0}' "$FILE"))
                            OptimizedTimes=$(cut -d "=" -f2 <<< $(awk '/User Optimization Overheads::Optimization phase time vs/ {printf "%s",$0}' "$FILE"))

                            entry="$procs,$threads,$gnx,$gny,$gnz,$npx,$npy,$npz,$nx,$ny,$nz"
                            entry="$entry,$ValidBench,$Rating,$ConvergenceTest,$SymmetryTest,$IterationCountTest,$ReproducibilityTest"
                            entry="$entry,$ConvergenceTestIterCount,$ConvergenceTestExpIterCount,$SymmetryTestSpmv"
                            entry="$entry,$IterationCountTestRefCGIter,$IterationCountTestOptCGIter,$IterationCountTestRefIter"
                            entry="$entry,$IterationCountTestOptIter,$ReproducibilityTestMean,$ReproducibilityTestVar"
                            entry="$entry,$LinearSize,$LinearNnz,$Format,$Rows,$Columns,$NonZeros"
                            entry="$entry,$TimeTotal,$TimeOpt,$TimeDot,$TimeWaxpby,$TimeSpmv,$TimeMG"
                            entry="$entry,$FPTotal,$FPDot,$FPWaxpby,$FPSpmv,$FPMG"
                            entry="$entry,$BWTotal,$BWRead,$BWWrite"
                            entry="$entry,$AITotal,$AIDot,$AIWaxpby,$AISpmv,$AIMG"
                            entry="$entry,$SetupTime,$MemoryData,$MemoryOpt,$OptOverheadsTime,$OptimizedTimes"

                            echo "$entry" 2>&1 | tee -a "$RESULTS_FILE"
                        fi
                    done
                done
            fi
        done
    fi
done
