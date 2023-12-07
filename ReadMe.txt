
mpiexec -np 1 env OMP_NUM_THREADS=16 /workspace/hpl.sh --dat HPL-dgx-4N.dat

mpirun --mca btl smcuda,self -x UCX_TLS=sm,cuda,cuda_copy,cuda_ipc -np 4 /workspace/hpcg.sh --dat ./hpcg.dat  --cpu-affinity 0-31:0-3
1:32-63:32-63

mpirun --mca btl smcuda,self -x UCX_TLS=sm,cuda,cuda_copy,cuda_ipc -np 1 env OMP_NUM_THREADS=16 /workspace/hpl.sh --dat HPL-dgx-4N.dat

export LD_LIBRARY_PATH=/usr/lib64:$LD_LIBRARY_PATH

singularity pull docker://nvcr.io/nvidia/hpc-benchmarks:23.10

mpiexec -np 4 env OMP_NUM_THREADS=16 /workspace/hpl.sh --dat HPL-dgx-4N.dat

