''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
       Intel(R) Optimized High Performance Conjugate Gradient Benchmark
................................................................................

Package Contents
----------------

This package contains Intel(R) Optimized High Performance Conjugate Gradient
Benchmark (Intel(R) Optimized HPCG) for Linux* OS.
The package is based on the reference implementation of the HPCG benchmark v3.1
with the following additions:

    setup/Make.*
      - Setup files for building a benchmark against optimizations provided
        in Intel(R) oneAPI Math Kernel Library (oneMKL).  The build steps are
        described in the HPCG QUICKSTART guide.

    bin/xhpcg_avx
      - Prebuilt executable with HPCG* subroutines optimized for Intel(R)
        Advanced Vector Extensions (Intel(R) AVX).

    bin/xhpcg_avx2
      - Prebuilt executable with HPCG* subroutines optimized for Intel(R)
        Advanced Vector Extensions 2 (Intel(R) AVX2).

    bin/xhpcg_knl
      - Prebuilt executable with HPCG* subroutines optimized for Intel(R)
        Advanced Vector Extensions 512 (Intel(R) AVX512) on Intel(R)
        Xeon Phi(TM) processors (products formerly Knights Landing).

    bin/xhpcg_skx
      - Prebuilt executable with HPCG* subroutines optimized for Intel(R)
        Advanced Vector Extensions 512 (Intel(R) AVX512) on Intel(R)
        Xeon(R) Scalable processors (products formerly Skylake).

For user convenience the package is supplied with prebuilt HPCG* launchers,
allowing out-of-the-box benchmark execution.  First, setup the runtime
environment for Intel(R) C Compiler/Intel(R) C++ Compiler, oneMKL, and
Intel(R) MPI Library using the scripts compilervars.sh, mklvars.sh,
and mpivars.sh included in those distributions.  Next, run the benchmark on
Intel AVX, Intel AVX2, or Intel AVX-512 enabled Intel processors as follows:

    #> export OMP_NUM_THREADS=<N_cores_per_node/N_processes_per_node>
    #> export KMP_AFFINITY=granularity=fine,compact,1,0
    #> mpiexec.hydra -genvall -n $nprocs -ppn $nprocs_per_node bin/xhpcg_avx  -n$problem_size -t$run_time_in_seconds
    #> mpiexec.hydra -genvall -n $nprocs -ppn $nprocs_per_node bin/xhpcg_avx2 -n$problem_size -t$run_time_in_seconds
    #> mpiexec.hydra -genvall -n $nprocs -ppn $nprocs_per_node bin/xhpcg_knl  -n$problem_size -t$run_time_in_seconds
    #> mpiexec.hydra -genvall -n $nprocs -ppn $nprocs_per_node bin/xhpcg_skx  -n$problem_size -t$run_time_in_seconds

Note: Problem size should be multiple of 8 and at least 24.

Reference HPCG code in the ./src directory was modified in order
to bundle Intel(R) architecture optimizations with the shared libraries.

===============================================================================
  Intel and the Intel logo are trademarks of Intel Corporation in the U.S.
  and/or other countries.

* Other names and brands may be claimed as the property of others.
================================================================================
Copyright 2014-2022 Intel Corporation.

This software and the related  documents are Intel  copyrighted  materials,  and
your use  of them  is governed  by the  express  license under  which  they were
provided to you (License).  Unless the  License provides otherwise,  you may not
use,  modify, copy,  publish, distribute,  disclose or transmit this software or
the related documents without Intel's prior written permission.

This software and the  related documents are provided as is,  with no express or
implied warranties, other than those that are expressly stated in the License.
================================================================================

 -- High Performance Conjugate Gradients (HPCG) Benchmark
    HPCG - 3.1 - March 28, 2019

    Michael A. Heroux
    Scalable Algorithms Group, Center for Computing Research
    Sandia National Laboratories, Albuquerque, NM

    Piotr Luszczek
    Jack Dongarra
    University of Tennessee, Knoxville
    Innovative Computing Laboratory
    (C) Copyright 2013-2019 All Rights Reserved

 -- Copyright notice and Licensing terms:

 Redistribution  and  use in  source and binary forms, with or without
 modification, are  permitted provided  that the following  conditions
 are met:

 1. Redistributions  of  source  code  must retain the above copyright
 notice, this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce  the above copyright
 notice, this list of conditions,  and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. The name of the  University,  the name of the  Laboratory,  or the
 names  of  its  contributors  may  not  be used to endorse or promote
 products  derived   from   this  software  without  specific  written
 permission.

 -- Disclaimer:

 THIS  SOFTWARE  IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY
 OR  CONTRIBUTORS  BE  LIABLE FOR ANY  DIRECT,  INDIRECT,  INCIDENTAL,
 SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES  (INCLUDING,  BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA OR PROFITS; OR BUSINESS INTERRUPTION)  HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT,  STRICT LIABILITY,  OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
======================================================================
