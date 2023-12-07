# HPCG-Basic-Tutorial

# HPCG Basic Tutorial

## Overview

This document guides the HPC HPCG group in setting up the environment and running the HPCG benchmark test.

## Background

Since we will be using Intel hardware in SCC. so all the examples are based on Intel arch. If there is anything you want to change or add, please send it directly to the hpcg group in slack.

**Support by** Professor Wang, Professor Eller and Professor Chung.

**Ref Link:**

*[NYU VPN](https://sites.google.com/nyu.edu/nyu-hpc/accessing-hpc/getting-and-renewing-an-account?authuser=0)*

*[HPCG ZOOM Recording](https://nyu.zoom.us/rec/play/MZPE3Y5ZQ0R6TiMGRGDtr0Y_dqQEC6gyVd9bF7YfOEBIowLQ5wmkdpRpYs9E3DmfDHtBCylWhPpjlUxt.hj8pVX6n4Z5vS8Qd?canPlayFromShare=true&from=share_recording_detail&continueMode=true&componentName=rec-play&originRequestUrl=https%3A%2F%2Fnyu.zoom.us%2Frec%2Fshare%2FLRpgw9qdOjhOj0sWqtpeesQMQX_XzrciITTkijWKlsOHSWGC3beZe9yWwGb3TBaD.vD6L9q7Fqf7lel88)*

*[HPCG Summary](https://www.lanl.gov/projects/crossroads/_assets/docs/ssi/Summary_HPCG.pdf)*

*[UL HPC Tutorials](https://ulhpc-tutorials.readthedocs.io/en/latest/parallel/hybrid/HPCG/)*

## Glossary

MPI_ICPC_OMP is a compiler and option. The structure is MPI + compiler + Thread(OMP)

```
ICPC: ICPC is Intel C++ Compiler, an optimized C/C++ compiler provided by Intel. This compiler can be used in conjunction with OpenMP and MPI to take full advantage of the parallelism on the Intel architecture.

MPI (Message Passing Interface): Imagine a neighborhood where each household is at a different location. When two residents want to exchange information, they can't just walk directly into each other's homes. Instead, they need to send letters or make phone calls. In the computational world, each process has its own separate address space. When they want to exchange data, they use a message-passing mechanism. This is similar to exchanging information over long distances, such as across different computational nodes and networks.

OMP (OpenMP): It's a parallelization based on threading. Imagine all threads living in the same big house. When one thread refers to "home", the other threads instantly know what it's referring to, because they all share the same address space. Inside this big house, communication between threads is very quick and direct.

When to use: Use MPI for communication between multiple processes running on different machines. On each server or computational node, to further parallelize tasks internally, use OpenMP.
```

## Architecture

1. Setup VPN if need
2. Download hpcg code
3. Edit Makefile as new hardware be used
4. Run hpcg and view results
5. Try to optimise it

## Components

### VPN

If you are within the NYU network, for example, if you’re on campus connected to the NYU Wi-Fi, skip this section.

However, if you’re **outside of the NYU network**, you must connect using the NYU VPN or Use NYU HPC gateway.

```bash
$sudo openconnect -b vpn.nyu.edu
```

username: your-NYU_NetID

password: your-NYU-password

password: push/sms

```bash
# Connect to gateway
$ssh <NetID>@gw.hpc.nyu.edu
```

Connect you to Greene HPC cluster (type in your-NYU-password )

```bash
# this will connect you to Greene HPC cluster
$ssh <NYU_NetID>@greene.hpc.nyu.edu
```

### Ensure you are logged into HPC (NetId@logX ~)!

### Download HPCG version **3.1** and extract it:

```bash
$ wget http://www.hpcg-benchmark.org/downloads/hpcg-3.1.tar.gz

$ tar xvzf hpcg-3.1.tar.gz

```

### Modify the Makefile, Configure, and Make

For understanding the next steps, the INSTALL file is a useful reference.

```bash
$ cd hpcg-3.1
$ nano INSTALL
```

1. First, load openmpi/intel/4.0.5:

```bash
$ module load openmpi/intel/4.0.5
```

1. Create a **build** directory:

```bash
$ mkdir build
$ cd build
```

1. Configure MPI_ICPC_OMP:
2. Since the file is for an older version and we’re using newer Intel hardware, the *Makefile* needs modification.
    
    ```bash
    $ cd ../setup/
    $ nano Make.MPI_ICPC_OMP
    
    ```
    
    Edit as follows:
    
    ```bash
    CHANGE
    
    CXXFLAGS = $(HPCG_DEFS) -O3 -openmp -mavx
    
    TO
    
    CXXFLAGS = $(HPCG_DEFS) -O3 -qopenmp -mavx2
    
    ```
    
3. To add debug output (disable this in a production environment):
    
    ```bash
     $ cp Make.MPI_ICPC_OMP Make.MPI_ICPC_OMP_DEBUG
     $ nano Make.MPI_ICPC_OMP_DEBUG
    
    ```
    
    Edit:
    
    ```bash
    CHANGE
    
    HPCG_OPTS     =
    
    TO
    
    HPCG_OPTS     =  -DHPCG_DEBUG
    ```
    
4. After running the command below, several files will be generated:
    1. Run in Build folder
    
    ```bash
    $ ../configure MPI_ICPC_OMP
    $ make
    ```
    
5. If there’s a **hpcg** program within the **bin** directory, it means successful generation.
    
    ```bash
    $ ls bin
    The output should contain "hpcg".
    ```
    

### Run hpcg

1. Navigate to the **bin** directory where **hpcg** is located:
    
    ```bash
    $ cd bin
    ```
    
2. To run **hpcg** on a single computing node with 48 MPI tasks (across 48 cores) using 180GB of memory: And *srun* is run the task with slurm job scheduler
    
    ```bash
     srun --nodes=1 --tasks-per-node=48 --mem=180GB xhpcg
    ```
    
    To monitor CPU or memory usage, SSH into the computing node:
    
    ```bash
    $ ssh csXX
    $ top
    ```
    
3. To view the results:
    
    ```bash
     nano HPCG-Benchmark_3.1_XXXXXX.txt
    ```
    

### Parameter Tuning:

1. Use multiple cores:

```bash
srun --nodes=1 --tasks-per-node=12 --cpus-per-task=4 --mem=180GB xhpcg
```

1. Use multiple nodes: (Note: Data communication goes through the infinite band.)

```bash
srun --nodes=4 --tasks-per-node=48 --mem=180GB xhpcg
```

```bash
srun --nodes=4 --tasks-per-node=12 --cpus-per-task=4 --mem=180GB xhpcg
```

```bash
srun --nodes=4 --tasks-per-node=24 --cpus-per-task=2 --mem=180GB xhpcg
```

```bash
srun -w cs003 --nodes=1 --tasks-per-node=12 --cpus-per-task=4 --mem=180GB xhpcg
```

### FAQ

1. mpicxx: Command not found

```bash
mpicxx -c -DHPCG_DEBUG -I./src -I./src/MPI_ICPC_OMP_DEBUG  -O3 -qopenmp -mavx2 -I../src ../src/main.cpp -o src/main.o
make: mpicxx: Command not found
make: *** [Makefile:64: src/main.o] Error 127
```

If you are disconnected from the HPC’s login node (e.g., you find yourself back in the gateway), you will need to connect to the HPC again. And load the module containing openmpi again.