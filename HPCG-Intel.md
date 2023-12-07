# HPCG Intel

### Intel HPCG tutorial: [https://www.intel.com/content/www/us/en/docs/onemkl/developer-guide-linux/2023-1/getting-started-with-intel-optimized-hpcg.html](https://www.intel.com/content/www/us/en/docs/onemkl/developer-guide-linux/2023-1/getting-started-with-intel-optimized-hpcg.html)

## Required Library:

Since Intel optimized HPCG relies on Intel's MKL math library, we need to install Intel OneAPI toolkits, and since Intel HPCG source code exists in the benchmark in the Intel HPC package, Intel HPC library is also required.

link for Intel OneAPI base: [https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html](https://www.intel.com/content/www/us/en/developer/tools/oneapi/base-toolkit-download.html)

link for Intel HPC Library: [https://www.intel.com/content/www/us/en/developer/tools/oneapi/hpc-toolkit.html#gs.741iim](https://www.intel.com/content/www/us/en/developer/tools/oneapi/hpc-toolkit.html#gs.741iim)

## Run:

Now we can start run the Intel optmized HPC.

1. go to the target folder where hpcg folder located.

```bash
# may be changed 
cd intel/mkl/latest/benchmarks/hpcg
```

1. Make a dir for compile, easy for compile command.

```bash
mkdir build
cd build/
```

1. Configure
    1. To maximize Intel CPU capacity, try to use AVX512 instruction set. (we will define it by bash)
    2. as we use OPENMPI, choose OPENMPI

```bash
../configure OPENMPI_IOMP_AVX2
```

1. Make a softlink to link the OPENMPI_IOMP_AVX2 into build folder.

```bash
cd setup
ln -s ../../setup/Make.OPENMPI_IOMP_AVX2 .
```

1. Make HPCG by bash

```bash
cd ../build
bash ../build-hpcg.bash make
```

1. Run by slurm

```bash
cd bin
srun --nodes=4 --tasks-per-node=12 --cpus-per-task=4 --mem=180GB xhpcg
```

**Here is the bash, which provided by Professor Shenglong**

```bash
#!/bin/bash

shopt -s expand_aliases
alias die='_error "Error in file $0 at line $LINENO:"'
alias warn='_warn "Warning in file $0 at line $LINENO:"'

function setup_cuda_compilers()
{
  local nvcc_version="11.0.2"
  module load cuda/${nvcc_version}
  export NVCC_PATH=$INTEL_WRAPPER_PATH/cuda/${nvcc_version}/bin
  export NVCC_BIN_PATH=$(dirname $(which nvcc))
}

function special_rules()
{
  return
  
  local arg=
  for arg in "$@"; do
    echo $arg
  done
}

function main()
{
  source /share/apps/lmod/lmod/init/profile
  export LMOD_DISABLE_SAME_NAME_AUTOSWAP=yes
  module use /share/apps/modulefiles
  module purge

  export PKG_CONFIG_PATH=
  export CPATH=
  export LIBRARY_PATH=
  export LD_LIBRARY_PATH=

  module load intel/19.1.2
  module load openmpi/intel/4.0.5

  export INTEL_WRAPPER_PATH="/share/apps/utils/intel"

  #setup_cuda_compilers

  local util=$INTEL_WRAPPER_PATH/util.bash
  if [ -e $util ]; then source $util; fi
  
  export SPECIAL_RULES_FUNCTION=special_rules
  if [ "$SPECIAL_RULES_FUNCTION" != "" ]; then
    export BUILD_WRAPPER_SCRIPT=$(readlink -e $0)
  fi

  export GNU_BIN_PATH=$(dirname $(which gcc))
  export INTEL_BIN_PATH=$(dirname $(which icc))
  export INTEL_MPI_BIN_PATH=$(dirname $(which mpicc))

  export INVALID_FLAGS="-O -O0 -O1 -O2 -O3 -g -g0 -openmp"
  
  export INVALID_FLAGS_FOR_GNU_COMPILERS=""
  export OPTIMIZATION_FLAGS_FOR_GNU_COMPILERS="-fPIC -fopenmp -mavx2"
  
  export INVALID_FLAGS_FOR_INTEL_COMPILERS="-lm -xhost -fast -msse2 -xCORE-AVX2"
  
  export OPTIMIZATION_FLAGS_FOR_INTEL_COMPILERS="-fPIC -unroll -ip -xCORE-AVX512 -qopenmp -qopt-report-stdout -qopt-report-phase=openmp"
  
  export OPTIMIZATION_FLAGS_FOR_INTEL_FORTRAN_COMPILERS="-fPIC -unroll -ip -xCORE-AVX512 -qopenmp -qopt-report-stdout -qopt-report-phase=openmp"

  #export INVALID_FLAGS_FOR_NVCC_COMPILERS=""
  #export OPTIMIZATION_FLAGS_FOR_NVCC_COMPILERS="-Wno-deprecated-gpu-targets"
  
  export OPTIMIZATION_FLAGS="-O3"
  
  export CPPFLAGS=$(for inc in $(env -u INTEL_INC -u MKL_INC | grep _INC= | cut -d= -f2); do echo '-I'$inc; done | xargs)
  export LDFLAGS=$(for lib in $(env | grep _LIB= | cut -d= -f2); do echo '-L'$lib; done | xargs)
  
  prepend_to_env_variable INCLUDE_FLAGS "$CPPFLAGS"
  prepend_to_env_variable LINK_FLAGS "$LDFLAGS"
  
  export INCLUDE_FLAGS_FOR_INTEL_COMPILERS="-I$INTEL_INC -I$MKL_INC"
  
  export LINK_FLAGS_FOR_INTEL_COMPILERS="-shared-intel"
  export EXTRA_LINK_FLAGS="$(LD_LIBRARY_PATH_to_rpath)"
  
  if [ "$DEBUG_LOG_FILE" != "" ]; then
    echo "" | tee $DEBUG_LOG_FILE
  fi
  
  export LD_RUN_PATH=$LD_LIBRARY_PATH

  local prefix=$(pwd)
  if [ "$prefix" == "" ]; then
    local dir=$(readlink -e $(dirname $0))
    dir="$dir/local"
    if [ -d $dir ]; then prefix=$dir; fi
  fi
  if [ "$prefix" == "" ]; then die "no prefix defined"; fi

  export HPCG_ILP64=yes

  #export N_MAKE_THREADS=60
  #export DEFAULT_COMPILER="GNU"
  
  local args="$@"
  local arg=
  for arg in $args; do
	
    case $arg in
	    
	    configure|conf)
        echo " Run configuration ..."
        export PATH=.:$INTEL_WRAPPER_PATH:$PATH
        
        if [ "$DEFAULT_COMPILER" != "GNU" ]; then
          export CC=icc
          export CXX=icpc
          export FC=ifort
          export F77=ifort
        fi
		
      ./configure --build=x86_64-centos-linux \
			    --prefix=$prefix
		;;
	    
	  cmake)
      module load cmake/3.18.4
      export PATH=.:$INTEL_WRAPPER_PATH:$PATH
      
      export CMAKE_INCLUDE_PATH=$(env | grep _INC= | cut -d= -f2 | xargs | sed -e 's/ /:/g')
      export CMAKE_LIBRARY_PATH=$(env | grep _LIB= | cut -d= -f2 | xargs | sed -e 's/ /:/g')
      
      export CC=icc
      export CXX=icpc
      cmake \
        -DCMAKE_BUILD_TYPE=release \
        -DBUILD_SHARED_LIBS::BOOL=ON \
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON \
        -DCMAKE_SKIP_RPATH:BOOL=ON \
        -DCMAKE_INSTALL_PREFIX:PATH=$prefix \
        ../breakdancer
    ;;
	    
	  make)
      export PATH=.:$INTEL_WRAPPER_PATH:$PATH
      echo " Run make"
      eval "$args" 
      exit
		;;

	  a2so)
      export PATH=.:$INTEL_WRAPPER_PATH:$PATH
      icc -shared -o libsuitesparse.so  \
        -Wl,--whole-archive \
        libamd.a \
        -Wl,--no-whole-archive \
        -L$MKL_ROOT/lib/intel64 -lmkl_rt
      exit
		;;
	    
	    *)
		die "Usage: $0 <argument>: configure make"
		;;
	esac

	args=$(eval "echo $args | sed -e 's/$arg //'")
    done
}

#############################################################
# do the main work here, do not modify the follwoing part,  #
# we just need to modify the main function                  #
#############################################################

if [ "$TO_SOURCE_BUILD_WRAPPER_SCRIPT" == "" ]; then
  main "$@"
  exit
else
  unset -f main
fi

#############################################################
# End here, do not add anything after this line             #
#############################################################
```

### Please check following code to meet the envir require

Module

```bash
module use /share/apps/modulefiles
```

FLAG

```bash
 export OPTIMIZATION_FLAGS_FOR_GNU_COMPILERS="-fPIC -fopenmp -mavx2"
  
  export INVALID_FLAGS_FOR_INTEL_COMPILERS="-lm -xhost -fast -msse2 -xCORE-AVX2"
  
  export OPTIMIZATION_FLAGS_FOR_INTEL_COMPILERS="-fPIC -unroll -ip -xCORE-AVX512 -qopenmp -qopt-report-stdout -qopt-report-phase=openmp"
  
  export OPTIMIZATION_FLAGS_FOR_INTEL_FORTRAN_COMPILERS="-fPIC -unroll -ip -xCORE-AVX512 -qopenmp -qopt-report-stdout -qopt-report-phase=openmp"

```