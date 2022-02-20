#!/bin/sh
# submit.sh
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

CMD="$1"
MACHINE="$2"
COMPILER="$3"
MODEL="$4"
TARGET="$5"
OUTPATH="$6"
PROGRESS="$7"
NPX="$8"
NPY="$9"
NPZ="${10}"

REPS=5
NFMTS=3 # 0:COO 1:CSR 2:DIA

if [ "$COMPILER" == "gnu-10.2" ]; then
        module load gcc/10.2.0
fi

if [ "$TARGET" == "OpenMP" ];then
    if [ "$MACHINE" == "archer" ]; then
        THREADS=("1" "4" "8" "16" "32" "64")
    elif [ "$MACHINE" == "cirrus" ]; then
        if [ "$COMPILER" == "cuda-11.2" ]; then
            THREADS=("1" "4" "8" "16" "20")
        elif [ "$COMPILER" == "gnu-10.2" ]; then
            THREADS=("1" "4" "8" "16" "18")
        fi
    fi
else
    THREADS=("1")
fi

for threads in "${THREADS[@]}"
do
    echo -e "\tThreads::$thread" 2>&1 | tee -a "$PROGRESS"
    export OMP_NUM_THREADS="$threads"

    # TODO: Remove weak scaling dimensions until we have a functional distributed HPCG in order to fit in memory
    #nx=$(( NPX*threads ))
    #ny=$(( NPY*threads ))
    #nz=$(( NPZ*threads ))
    nx=$NPX
    ny=$NPY
    nz=$NPZ
	
    echo -e "Global Problem Size::$nx $ny $nz" 2>&1 | tee -a "$PROGRESS"
    for iter in $(seq 1 $REPS)
    do
        echo -e "\t\tRepetition::$iter" 2>&1 | tee -a "$PROGRESS"

        if [ "$MODEL" == "HPCG" ] || [ "$MODEL" == "Concrete" ]; then
                START_FMT=1
                END_FMT=1
        else
            START_FMT=0
            # FIXME: Skip COO until OpenMP is fixed
            if [ "$TARGET" == "OpenMP" ];then
                START_FMT=1
            fi
            END_FMT=$(( $NFMTS - 1 ))
        fi

        for fmt in $(seq $START_FMT $END_FMT)
        do  
            echo -e "\t\tFormat::$fmt" 2>&1 | tee -a "$PROGRESS"

            OUTDIR="$OUTPATH/$threads/$iter/$fmt"
            OUTFILE="$OUTDIR/out.txt"
           
	        mkdir -p $(dirname $OUTFILE)

            fmt_arg=""
            if [ "$model" != "HPCG" ]
            then
                fmt_arg="--format=$fmt"
            fi
	   
	        ARGS="--nx=${nx} --ny=${ny} --nz=${nz} $fmt_arg"
	        cd $OUTDIR	# needed such that the HPCG out files are saved in there

            $CMD $ARGS && mv "$OUTDIR/HPCG-Benchmark*" $OUTFILE
        done
    done
done
