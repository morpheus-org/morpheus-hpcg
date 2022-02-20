#!/bin/bash

# Slurm job options (name, compute nodes, job time)
#SBATCH --job-name=Example_MPI_Job
#SBATCH --time=0:5:0
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --tasks-per-node=36
#SBATCH --cpus-per-task=1
#SBATCH --account=e609
#SBATCH --partition=standard
#SBATCH --qos=short

# Load the default HPE MPI environment
module load mpt

. ~/.bashrc
load_env release gnu-10.2 on off

# Change to the submission directory
cd $SLURM_SUBMIT_DIR
export OMP_NUM_THREADS=1

srun --ntasks=1 --tasks-per-node=1 --cpus-per-task=1 --hint=nomultithread ../../build-Release-gnu-10.2-MPI/xhpcg
srun --ntasks=2 --tasks-per-node=2 --cpus-per-task=1 --hint=nomultithread ../../build-Release-gnu-10.2-MPI/xhpcg
srun --ntasks=3 --tasks-per-node=3 --cpus-per-task=1 --hint=nomultithread ../../build-Release-gnu-10.2-MPI/xhpcg
srun --ntasks=4 --tasks-per-node=4 --cpus-per-task=1 --hint=nomultithread ../../build-Release-gnu-10.2-MPI/xhpcg