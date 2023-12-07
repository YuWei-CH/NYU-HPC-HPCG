# HPC Nvidia

Basically, we'll use 4 A100 GPUs to run HPCG, where "HPCG" refers to the Nvidia optimized version, and we only have the binary file so there's no way to recompile it from scratch. 

Link for Nvidia HPC: [https://catalog.ngc.nvidia.com/orgs/nvidia/containers/hpc-benchmarks](https://catalog.ngc.nvidia.com/orgs/nvidia/containers/hpc-benchmarks)

**Note: Using the Nvidia optimized version requires the use of singularity.**

1. Reserve 4 GPU node

```bash
# reserve GPU node
srun --cpus-per-task=64 --threads-per-core=1 --mem=400GB --time=04:00:00 --exclusive --gres=gpu:a100:4 --pty /bin/bash
```

1. Since the **slurm** in the GPU Node will conflict with **mpirun**. When the mpirun tries to create 4 processes, the **slurm** will block this request, resulting in only one process being available. So we need to **remove the slurm** in GPU Node first.

```bash
# remove slurm
for e in $(env | egrep ^SLURM_ | cut -d= -f1); do unset $e; done
```

1. Go to the target folder, where the hpcg.dat exist.

```bash
# go to target folder(can be change)
cd $SCRATCH/test
```

1. Start singularity. Do not forget to add **--nv** flag. The location of singularity should be changed in SCC23 cluster.

```bash
# singularity
singularity exec --nv /scratch/work/public/singularity/nvidia-hpc-benchmarks-23.5.sif /bin/bash
```

1. MPIRUN. 1. 4 process;  2. 16 cpu tasks per process; 

```bash
# run
mpirun --mca btl smcuda,self -x UCX_TLS=sm,cuda,cuda_copy,cuda_ipc -np 4 /workspace/hpcg.sh --dat ./hpcg.dat  --cpu-affinity 0-15:16-31:32-47:48-63
```

We can check gpu status by following command.

```bash
# go to ga004
[wang@ga004 ~]$ for p in $(pidof xhpcg); do taskset -pc $p; done
pid 799656's current affinity list: 48-63
pid 799653's current affinity list: 0-15
pid 799652's current affinity list: 32-47
pid 799651's current affinity list: 16-31

+---------------------------------------------------------------------------------------+
| Processes:                                                                            |
|  GPU   GI   CI        PID   Type   Process name                            GPU Memory |
|        ID   ID                                                             Usage      |
|=======================================================================================|
|    0   N/A  N/A    799653      C   /workspace/hpcg-linux-x86_64/xhpcg        16472MiB |
|    1   N/A  N/A    799651      C   /workspace/hpcg-linux-x86_64/xhpcg        16472MiB |
|    2   N/A  N/A    799652      C   /workspace/hpcg-linux-x86_64/xhpcg        16472MiB |
|    3   N/A  N/A    799656      C   /workspace/hpcg-linux-x86_64/xhpcg        16472MiB |
+---------------------------------------------------------------------------------------+

# update smi per 4 second 
nvidia-smi -l 4

/scratch/work/public/singularity/run-nvtop-3.0.0.bash nvtop
```

## **According to test, the best size for HPCG is 512 512 256, and we should run it at least 1900 second.**