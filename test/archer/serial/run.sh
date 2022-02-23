#!/bin/bash
#SBATCH --job-name=hpcg-serial
#SBATCH --time=0:20:0
#SBATCH --nodes=1
#SBATCH --tasks-per-node=128
#SBATCH --cpus-per-task=1

#SBATCH --account=e609             
#SBATCH --partition=standard
#SBATCH --qos=standard

export OMP_NUM_THREADS=1

HPCG_EXE_PATH="/work/e609/e609/cstyl/morpheus-hpcg/build-Release-Cray-MPI-dynamic-serial"

srun --distribution=block:block --hint=nomultithread ${HPCG_EXE_PATH}/morpheus-hpcg
