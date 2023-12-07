$ wget http://www.hpcg-benchmark.org/downloads/hpcg-3.1.tar.gz
$ tar xvzf hpcg-3.1.tar.gz

$ cd hpcg-3.1
$ nano INSTALL

$ module load openmpi/intel/4.0.5

$ mkdir build
$ cd build

$ cd ../setup/
$ nano Make.MPI_ICPC_OMP

(
    CHANGE

CXXFLAGS = $(HPCG_DEFS) -O3 -openmp -mavx

TO

CXXFLAGS = $(HPCG_DEFS) -O3 -qopenmp -mavx2
)

 $ cp Make.MPI_ICPC_OMP Make.MPI_ICPC_OMP_DEBUG
 $ nano Make.MPI_ICPC_OMP_DEBUG

 (

CHANGE

HPCG_OPTS     =

TO

HPCG_OPTS     =  -DHPCG_DEBUG

 )

$ ../configure MPI_ICPC_OMP
$ make

$ cd bin

$ srun --nodes=1 --tasks-per-node=48 --mem=180GB xhpcg
$ srun --nodes=4 --tasks-per-node=12 --cpus-per-task=4 --mem=180GB xhpcg