/*******************************************************************************
* Copyright 2014-2022 Intel Corporation.
*
* This software and the related documents are Intel copyrighted  materials,  and
* your use of  them is  governed by the  express license  under which  they were
* provided to you (License).  Unless the License provides otherwise, you may not
* use, modify, copy, publish, distribute,  disclose or transmit this software or
* the related documents without Intel's prior written permission.
*
* This software and the related documents  are provided as  is,  with no express
* or implied  warranties,  other  than those  that are  expressly stated  in the
* License.
*******************************************************************************/

//@HEADER
// ***************************************************
//
// HPCG: High Performance Conjugate Gradient Benchmark
//
// Contact:
// Michael A. Heroux ( maherou@sandia.gov)
// Jack Dongarra     (dongarra@eecs.utk.edu)
// Piotr Luszczek    (luszczek@eecs.utk.edu)
//
// ***************************************************
//@HEADER

/*!
 @file hpcg.hpp

 HPCG data structures and functions
 */

#ifndef HPCG_HPP
#define HPCG_HPP

#include <fstream>
#include "Geometry.hpp"

//#define DBL_EPSILON 2.2204460492503131E-16
#define fabs_hpcg(a) ((a) >= 0.0) ? (a) : -(a)

extern std::ofstream HPCG_fout;

struct HPCG_Params_STRUCT {
  int comm_size; //!< Number of MPI processes in MPI_COMM_WORLD
  int comm_rank; //!< This process' MPI rank in the range [0 to comm_size - 1]
  int numThreads; //!< This process' number of threads
  local_int_t nx; //!< Number of x-direction grid points for each local subdomain
  local_int_t ny; //!< Number of y-direction grid points for each local subdomain
  local_int_t nz; //!< Number of z-direction grid points for each local subdomain
  int runningTime; //!< Number of seconds to run the timed portion of the benchmark
  int npx; //!< Number of processes in x-direction of 3D process grid
  int npy; //!< Number of processes in y-direction of 3D process grid
  int npz; //!< Number of processes in z-direction of 3D process grid
  int pz; //!< Partition in the z processor dimension, default is npz
  local_int_t zl; //!< nz for processors in the z dimension with value less than pz
  local_int_t zu; //!< nz for processors in the z dimension with value greater than pz
  int runRealRef;  // default true, turn on reference implementation
  char yamlFileName[1024];
 
};
/*!
  HPCG_Params is a shorthand for HPCG_Params_STRUCT
 */
typedef HPCG_Params_STRUCT HPCG_Params;

extern int HPCG_Init(int * argc_p, char ** *argv_p, HPCG_Params & params);
extern int HPCG_Finalize(void);

#endif // HPCG_HPP
