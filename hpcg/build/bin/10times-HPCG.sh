#!/bin/bash

#SBATCH --cpus-per-task=64
#SBATCH --threads-per-core=1
#SBATCH --mem=400GB
#SBATCH --time=08:00:00
#SBATCH --gres=gpu:a100:4
#SBATCH --exclusive
#SBATCH --job-name=10_HPCG_job
#SBATCH --mail-type=ALL
#SBATCH --mail-user=ys4680@nyu.edu

# load openMPI module
module load openmpi/intel/4.1.1

# remove slurm
for e in $(env | egrep ^SLURM_ | cut -d= -f1); do unset $e; done

# go to target folder(change)
cd /scratch/work/scc23/group/ys4680/benchmarks/hpcg/build/bin

for i in {1..10}; do
    echo "Run $i" >> results.txt
    # start singularity to mpirun HPCG
    singularity exec --nv /scratch/work/public/singularity/nvidia-hpc-benchmarks-23.5.sif mpirun --mca btl smcuda,self -x UCX_TLS=sm,cuda,cuda_copy,cuda_ipc -np 4 /workspace/hpcg.sh --dat ./hpcg.dat  --cpu-affinity 0-15:16-31:32-47:48-63 >> results.txt
    echo "===========" >> results.txt
done
