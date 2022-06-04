#!/bin/sh
# parse-hpcg-out.sh
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

create_hpcg_header() {
    header="procs,threads,gnx,gny,gnz,npx,npy,npz,nx,ny,nz"
    header="$header,ValidBench,Rating,ConvergenceTest,SymmetryTest,IterationCountTest,ReproducibilityTest"
    header="$header,ConvergenceTestIterCount,ConvergenceTestExpIterCount,SymmetryTestSpmv"
    header="$header,IterationCountTestRefCGIter,IterationCountTestOptCGIter,IterationCountTestRefIter"
    header="$header,IterationCountTestOptIter,ReproducibilityTestMean,ReproducibilityTestVar"
    header="$header,LinearSize,LinearNnz"
    header="$header,TimeTotal,TimeOpt,TimeDot,TimeWaxpby,TimeSpmv,TimeMG"
    header="$header,FPTotal,FPDot,FPWaxpby,FPSpmv,FPMG,FPTotalWithConv"
    header="$header,BWTotal,BWRead,BWWrite,BWTotalWithConv"
    header="$header,FlopRateTotal,FlopRateDot,FlopRateWaxpby,FlopRateSpmv,FlopRateMG"
    header="$header,FlopRateWithConv,FlopRateFinal,FlopRateHistorical"
    header="$header,SetupTime,MemoryData,MemoryOpt,MemoryBytesPerEq,MemoryLinearSystem,OptOverheadsTime,OptimizedTimes"

    echo $header
}

parse_hpcg_file() {
    filename=$1

    procs=$(cut -d "=" -f2 <<<$(awk '/Machine Summary::Distributed/ {printf "%s",$0}' "$filename"))
    threads=$(cut -d "=" -f2 <<<$(awk '/Machine Summary::Threads/ {printf "%s",$0}' "$filename"))
    gnx=$(cut -d "=" -f2 <<<$(awk '/Global Problem Dimensions::Global nx/ {printf "%s",$0}' "$filename"))
    gny=$(cut -d "=" -f2 <<<$(awk '/Global Problem Dimensions::Global ny/ {printf "%s",$0}' "$filename"))
    gnz=$(cut -d "=" -f2 <<<$(awk '/Global Problem Dimensions::Global nz/ {printf "%s",$0}' "$filename"))
    npx=$(cut -d "=" -f2 <<<$(awk '/Processor Dimensions::npx/ {printf "%s",$0}' "$filename"))
    npy=$(cut -d "=" -f2 <<<$(awk '/Processor Dimensions::npy/ {printf "%s",$0}' "$filename"))
    npz=$(cut -d "=" -f2 <<<$(awk '/Processor Dimensions::npz/ {printf "%s",$0}' "$filename"))
    nx=$(cut -d "=" -f2 <<<$(awk '/Local Domain Dimensions::nx/ {printf "%s",$0}' "$filename"))
    ny=$(cut -d "=" -f2 <<<$(awk '/Local Domain Dimensions::ny/ {printf "%s",$0}' "$filename"))
    nz=$(cut -d "=" -f2 <<<$(awk '/Local Domain Dimensions::nz/ {printf "%s",$0}' "$filename"))

    ConvergenceTest=$(cut -d "=" -f2 <<<$(awk '/Spectral Convergence Tests::Result/ {printf "%s",$0}' "$filename"))
    SymmetryTest=$(cut -d "=" -f2 <<<$(awk '/^Departure from Symmetry.*Result/ {printf "%s",$0}' "$filename"))
    IterationCountTest=$(cut -d "=" -f2 <<<$(awk '/Iteration Count Information::Result/ {printf "%s",$0}' "$filename"))
    ReproducibilityTest=$(cut -d "=" -f2 <<<$(awk '/Reproducibility Information::Result/ {printf "%s",$0}' "$filename"))
    ValidBench=$(awk '/Final Summary::HPCG result/ {printf "%s",$5}' "$filename")

    ConvergenceTestIterCount=$(cut -d "=" -f2 <<<$(awk '/^Spectral Convergence Tests::Unpreconditioned::Maximum/ {printf "%s",$0}' "$filename"))
    ConvergenceTestExpIterCount=$(cut -d "=" -f2 <<<$(awk '/Spectral Convergence Tests::Unpreconditioned::Expected/ {printf "%s",$0}' "$filename"))
    SymmetryTestSpmv=$(cut -d "=" -f2 <<<$(awk '/^Departure from Symmetry.*Departure for SpMV/ {printf "%s",$0}' "$filename"))
    IterationCountTestRefCGIter=$(cut -d "=" -f2 <<<$(awk '/Iteration Count Information::Reference/ {printf "%s",$0}' "$filename"))
    IterationCountTestOptCGIter=$(cut -d "=" -f2 <<<$(awk '/Iteration Count Information::Optimized/ {printf "%s",$0}' "$filename"))
    IterationCountTestRefIter=$(cut -d "=" -f2 <<<$(awk '/^Iteration Count.*reference iterations/ {printf "%s",$0}' "$filename"))
    IterationCountTestOptIter=$(cut -d "=" -f2 <<<$(awk '/^Iteration Count.*optimized iterations/ {printf "%s",$0}' "$filename"))
    ReproducibilityTestMean=$(cut -d "=" -f2 <<<$(awk '/^Reproducibility Information.*mean/ {printf "%s",$0}' "$filename"))
    ReproducibilityTestVar=$(cut -d "=" -f2 <<<$(awk '/^Reproducibility Information.*variance/ {printf "%s",$0}' "$filename"))

    LinearSize=$(cut -d "=" -f2 <<<$(awk '/Linear System Information::Number of Equations/ {printf "%s",$0}' "$filename"))
    LinearNnz=$(cut -d "=" -f2 <<<$(awk '/Linear System Information::Number of Nonzero Terms/ {printf "%s",$0}' "$filename"))

    TimeTotal=$(cut -d "=" -f2 <<<$(awk '/Benchmark Time Summary::Total/ {printf "%s",$0}' "$filename"))
    TimeOpt=$(cut -d "=" -f2 <<<$(awk '/Benchmark Time Summary::Optimization/ {printf "%s",$0}' "$filename"))
    TimeDot=$(cut -d "=" -f2 <<<$(awk '/Benchmark Time Summary::DDOT/ {printf "%s",$0}' "$filename"))
    TimeWaxpby=$(cut -d "=" -f2 <<<$(awk '/Benchmark Time Summary::WAXPBY/ {printf "%s",$0}' "$filename"))
    TimeSpmv=$(cut -d "=" -f2 <<<$(awk '/Benchmark Time Summary::SpMV/ {printf "%s",$0}' "$filename"))
    TimeMG=$(cut -d "=" -f2 <<<$(awk '/Benchmark Time Summary::MG/ {printf "%s",$0}' "$filename"))

    FPTotal=$(cut -d "=" -f2 <<<$(awk '/Floating Point Operations Summary::Total=/ {printf "%s",$0}' "$filename"))
    FPDot=$(cut -d "=" -f2 <<<$(awk '/Floating Point Operations Summary::Raw DDOT/ {printf "%s",$0}' "$filename"))
    FPWaxpby=$(cut -d "=" -f2 <<<$(awk '/Floating Point Operations Summary::Raw WAXPBY/ {printf "%s",$0}' "$filename"))
    FPSpmv=$(cut -d "=" -f2 <<<$(awk '/Floating Point Operations Summary::Raw SpMV/ {printf "%s",$0}' "$filename"))
    FPMG=$(cut -d "=" -f2 <<<$(awk '/Floating Point Operations Summary::Raw MG/ {printf "%s",$0}' "$filename"))
    FPTotalWithConv=$(cut -d "=" -f2 <<<$(awk '/Floating Point Operations Summary::Total with convergence overhead/ {printf "%s",$0}' "$filename"))

    BWTotal=$(cut -d "=" -f2 <<<$(awk '/GB\/s Summary::Raw Total/ {printf "%s",$0}' "$filename"))
    BWRead=$(cut -d "=" -f2 <<<$(awk '/GB\/s Summary::Raw Read/ {printf "%s",$0}' "$filename"))
    BWWrite=$(cut -d "=" -f2 <<<$(awk '/GB\/s Summary::Raw Write/ {printf "%s",$0}' "$filename"))
    BWTotalWithConv=$(cut -d "=" -f2 <<<$(awk '/GB\/s Summary::Total with convergence and optimization phase overhead/ {printf "%s",$0}' "$filename"))

    FlopRateTotal=$(cut -d "=" -f2 <<<$(awk '/GFLOP\/s Summary::Raw Total/ {printf "%s",$0}' "$filename"))
    FlopRateDot=$(cut -d "=" -f2 <<<$(awk '/GFLOP\/s Summary::Raw DDOT/ {printf "%s",$0}' "$filename"))
    FlopRateWaxpby=$(cut -d "=" -f2 <<<$(awk '/GFLOP\/s Summary::Raw WAXPBY/ {printf "%s",$0}' "$filename"))
    FlopRateSpmv=$(cut -d "=" -f2 <<<$(awk '/GFLOP\/s Summary::Raw SpMV/ {printf "%s",$0}' "$filename"))
    FlopRateMG=$(cut -d "=" -f2 <<<$(awk '/GFLOP\/s Summary::Raw MG/ {printf "%s",$0}' "$filename"))
    FlopRateWithConv=$(cut -d "=" -f2 <<<$(awk '/GFLOP\/s Summary::Total with convergence overhead/ {printf "%s",$0}' "$filename"))
    FlopRateFinal=$(cut -d "=" -f2 <<<$(awk '/GFLOP\/s Summary::Total with convergence and optimization phase overhead/ {printf "%s",$0}' "$filename"))
    FlopRateHistorical=$(cut -d "=" -f2 <<<$(awk '/Final Summary::HPCG 2.4 rating for historical reasons is/ {printf "%s",$0}' "$filename"))

    SetupTime=$(cut -d "=" -f2 <<<$(awk '/Setup Information::Setup/ {printf "%s",$0}' "$filename"))
    MemoryData=$(cut -d "=" -f2 <<<$(awk '/Memory Use Information::Total memory used for data \(Gbytes\)/ {printf "%s",$0}' "$filename"))
    MemoryOpt=$(cut -d "=" -f2 <<<$(awk '/Memory Use Information::Memory used for OptimizeProblem data/ {printf "%s",$0}' "$filename"))
    MemoryBytesPerEq=$(cut -d "=" -f2 <<<$(awk '/Memory Use Information::Bytes per equation \(Total memory \/ Number of Equations\)/ {printf "%s",$0}' "$filename"))
    MemoryLinearSystem=$(cut -d "=" -f2 <<<$(awk '/Memory Use Information::Memory used for linear system and CG \(Gbytes\)/ {printf "%s",$0}' "$filename"))
    OptOverheadsTime=$(cut -d "=" -f2 <<<$(awk '/User Optimization Overheads::Optimization phase time \(sec\)/ {printf "%s",$0}' "$filename"))
    OptimizedTimes=$(cut -d "=" -f2 <<<$(awk '/User Optimization Overheads::Optimization phase time vs/ {printf "%s",$0}' "$filename"))

    entry="$procs,$threads,$gnx,$gny,$gnz,$npx,$npy,$npz,$nx,$ny,$nz"
    entry="$entry,$ValidBench,$ConvergenceTest,$SymmetryTest,$IterationCountTest,$ReproducibilityTest"
    entry="$entry,$ConvergenceTestIterCount,$ConvergenceTestExpIterCount,$SymmetryTestSpmv"
    entry="$entry,$IterationCountTestRefCGIter,$IterationCountTestOptCGIter,$IterationCountTestRefIter"
    entry="$entry,$IterationCountTestOptIter,$ReproducibilityTestMean,$ReproducibilityTestVar"
    entry="$entry,$LinearSize,$LinearNnz"
    entry="$entry,$TimeTotal,$TimeOpt,$TimeDot,$TimeWaxpby,$TimeSpmv,$TimeMG"
    entry="$entry,$FPTotal,$FPDot,$FPWaxpby,$FPSpmv,$FPMG,$FPTotalWithConv"
    entry="$entry,$BWTotal,$BWRead,$BWWrite,$BWTotalWithConv"
    entry="$entry,$FlopRateTotal,$FlopRateDot,$FlopRateWaxpby,$FlopRateSpmv,$FlopRateMG"
    entry="$entry,$FlopRateWithConv,$FlopRateFinal,$FlopRateHistorical"
    entry="$entry,$SetupTime,$MemoryData,$MemoryOpt,$MemoryBytesPerEq,$MemoryLinearSystem,$OptOverheadsTime,$OptimizedTimes"

    echo $entry
}
