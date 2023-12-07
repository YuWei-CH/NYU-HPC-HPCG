
https://infohub.delltechnologies.com/p/accelerating-hpc-workloads-with-nvidia-a100-nvlink-on-dell-poweredge-xe8545/

https://infohub.delltechnologies.com/p/hpc-application-performance-on-dell-poweredge-r7525-servers-with-nvidia-a100-gpgpus-1/

https://www.pugetsystems.com/labs/hpc/outstanding-performance-of-nvidia-a100-pcie-on-hpl-hpl-ai-hpcg-benchmarks-2149/

https://catalog.ngc.nvidia.com/orgs/nvidia/containers/hpc-benchmarks


singularity exec --nv /scratch/work/public/singularity/nvidia-hpc-benchmarks-23.5.sif /bin/bash

cp -rp /workspace/hpcg-linux-x86_64/sample-dat/hpcg.dat . 

#HPL

mpirun --mca btl smcuda,self -x UCX_TLS=sm,cuda,cuda_copy,cuda_ipc -np 2 hpl.sh --dat ./HPL.dat --cpu-affinity 0:0 --cpu-cores-per-rank 4 --gpu-affinity 0:1

#HPL-AI

mpirun --mca btl smcuda,self -x MELLANOX_VISIBLE_DEVICES="none" -x UCX_TLS=sm,cuda,cuda_copy,cuda_ipc -np 2 hpl.sh --xhpl-ai --dat ./HPL.dat --cpu-affinity 0:0 --cpu-cores-per-rank 4 --gpu-affinity 0:1

#HPCG

mpirun --mca btl smcuda,self -x UCX_TLS=sm,cuda,cuda_copy,cuda_ipc -np 4 /workspace/hpcg.sh --dat ./hpcg.dat  



mpirun --mca btl smcuda,self -x UCX_TLS=sm,cuda,cuda_copy,cuda_ipc -np 4 /workspace/hpcg.sh --dat ./hpcg.dat  --cpu-affinity 0-39:0-39:40-79:40-79 


[wang@ga043 ~]$ for p in $(pidof xhpcg); do taskset -pc $p; done
pid 1140918's current affinity list: 40-79
pid 1140917's current affinity list: 0-39
pid 1140912's current affinity list: 0-39
pid 1140897's current affinity list: 40-79
[wang@ga043 ~]$ 


Final Summary=
Final Summary::HPCG result is VALID with a GFLOP/s rating of=1098.35
Final Summary::HPCG 2.4 rating for historical reasons is=1102.84
Final Summary::Please upload results from the YAML file contents to=http://hpcg-benchmark.org


mpirun --mca btl smcuda,self -x UCX_TLS=sm,cuda,cuda_copy,cuda_ipc -np 4 /workspace/hpcg.sh --dat ./hpcg.dat  --cpu-affinity 0-31:0-31:32-63:32-63

