#!/bin/bash
#
#SBATCH --partition=gpu-cascade
#SBATCH --qos=gpu
#SBATCH --gres=gpu:4
#SBATCH --time=00:05:00

# Replace [budget code] below with your project code (e.g. t01)
#SBATCH --account=e609

# Load the required modules
module load mpt
# module load nvidia
# dev=3
# srun --ntasks=$dev --tasks-per-node=$dev cuda-memcheck ../build-Release-cuda-11.2-MPI-concrete-cuda/xhpcg --kokkos-num-devices=$dev --kokkos-numa=2
# srun --ntasks=$dev --tasks-per-node=$dev ../build-Release-cuda-11.2-MPI-concrete-cuda/xhpcg --kokkos-num-devices=$dev

export OMP_NUM_THREADS=1
export OMP_PLACES=cores

srun --ntasks=1 --tasks-per-node=1 --cpus-per-task=1 --hint=nomultithread ../../build-Release-cuda-11.2-MPI-dynamic-cuda/morpheus-hpcg --kokkos-num-devices=1 --kokkos-numa=2
srun --ntasks=2 --tasks-per-node=2 --cpus-per-task=1 --hint=nomultithread ../../build-Release-cuda-11.2-MPI-dynamic-cuda/morpheus-hpcg --kokkos-num-devices=2 --kokkos-numa=2
srun --ntasks=3 --tasks-per-node=3 --cpus-per-task=1 --hint=nomultithread ../../build-Release-cuda-11.2-MPI-dynamic-cuda/morpheus-hpcg --kokkos-num-devices=3 --kokkos-numa=2
srun --ntasks=4 --tasks-per-node=4 --cpus-per-task=1 --hint=nomultithread ../../build-Release-cuda-11.2-MPI-dynamic-cuda/morpheus-hpcg --kokkos-num-devices=4 --kokkos-numa=2

# # 1 GPU
# Benchmark Time Summary::Optimization phase=0.00334863
# Benchmark Time Summary::DDOT=0.131138
# Benchmark Time Summary::WAXPBY=0.000917887
# Benchmark Time Summary::SpMV=0.00039242
# Benchmark Time Summary::MG=0.00102632
# Benchmark Time Summary::Total=0.133523

# # 2 GPU
# Benchmark Time Summary::Optimization phase=0.00320552
# Benchmark Time Summary::DDOT=0.297825
# Benchmark Time Summary::WAXPBY=0.000983987
# Benchmark Time Summary::SpMV=0.000914901
# Benchmark Time Summary::MG=0.00110998
# Benchmark Time Summary::Total=0.300891

# # 3 GPU
# Benchmark Time Summary::Optimization phase=0.144452
# Benchmark Time Summary::DDOT=6.53667
# Benchmark Time Summary::WAXPBY=0.00149989
# Benchmark Time Summary::SpMV=0.00563186
# Benchmark Time Summary::MG=0.00136391
# Benchmark Time Summary::Total=6.5453

# # 4 GPU
# Benchmark Time Summary::Optimization phase=0.213327
# Benchmark Time Summary::DDOT=8.62051
# Benchmark Time Summary::WAXPBY=0.00150523
# Benchmark Time Summary::SpMV=0.011399
# Benchmark Time Summary::MG=0.00146904
# Benchmark Time Summary::Total=8.63502

# # --ntasks=1 --tasks-per-node=1 --cpus-per-task=1 --hint=nomultithread --kokkos-num-devices=1 --kokkos-numa=2
# # 1 GPU
# Benchmark Time Summary::Optimization phase=0.00374864
# Benchmark Time Summary::DDOT=0.112816
# Benchmark Time Summary::WAXPBY=0.000761765
# Benchmark Time Summary::SpMV=0.000317791
# Benchmark Time Summary::MG=0.000914876
# Benchmark Time Summary::Total=0.114855

# # 2 GPU
# Benchmark Time Summary::Optimization phase=0.00397523
# Benchmark Time Summary::DDOT=0.258162
# Benchmark Time Summary::WAXPBY=0.00080787
# Benchmark Time Summary::SpMV=0.000878115
# Benchmark Time Summary::MG=0.000987576
# Benchmark Time Summary::Total=0.260883

# # 3 GPU
# Benchmark Time Summary::Optimization phase=0.00406135
# Benchmark Time Summary::DDOT=0.323345
# Benchmark Time Summary::WAXPBY=0.000757773
# Benchmark Time Summary::SpMV=0.000866238
# Benchmark Time Summary::MG=0.000903213
# Benchmark Time Summary::Total=0.325919

# # 4 GPU
# Benchmark Time Summary::Optimization phase=0.00436608
# Benchmark Time Summary::DDOT=0.394356
# Benchmark Time Summary::WAXPBY=0.00071633
# Benchmark Time Summary::SpMV=0.000963318
# Benchmark Time Summary::MG=0.000869
# Benchmark Time Summary::Total=0.396983
