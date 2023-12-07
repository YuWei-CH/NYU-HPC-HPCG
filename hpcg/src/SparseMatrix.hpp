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
 @file SparseMatrix.hpp

 HPCG data structures for the sparse matrix
 */

#ifndef SPARSEMATRIX_HPP
#define SPARSEMATRIX_HPP
#define MKL

#include <vector>
#include <cassert>
#include "Geometry.hpp"
#include "Vector.hpp"
#include "MGData.hpp"
#if __cplusplus < 201103L
// for C++03
#include <map>
typedef std::map< global_int_t, local_int_t > GlobalToLocalMap;
#else
// for C++11 or greater
#include <unordered_map>
using GlobalToLocalMap = std::unordered_map< global_int_t, local_int_t >;
#endif

#include "mkl_spblas.h"
#include "mkl_service.h"
#include "stdio.h"

struct optData
{
    double *diag;
    double *dtmp;
    double *dtmp2;
    double *dtmp3;
    double *dtmp4;
    local_int_t nrow_b;
    local_int_t *bmap;
    void *csrA;
    void *csrB;
};

struct SparseMatrix_STRUCT {
  char  * title; //!< name of the sparse matrix
  Geometry * geom; //!< geometry associated with this matrix
  global_int_t totalNumberOfRows; //!< total number of matrix rows across all processes
  global_int_t totalNumberOfNonzeros; //!< total number of matrix nonzeros across all processes
  local_int_t localNumberOfRows; //!< number of rows local to this process
  local_int_t localNumberOfColumns;  //!< number of columns local to this process
  local_int_t localNumberOfNonzeros;  //!< number of nonzeros local to this process
  char  * nonzerosInRow;  //!< The number of nonzeros in a row will always be 27 or fewer
  global_int_t ** mtxIndG; //!< matrix indices as global values
  local_int_t ** mtxIndL; //!< matrix indices as local values
  double ** matrixValues; //!< values of matrix entries
  double ** matrixDiagonal; //!< values of matrix diagonal entries
  GlobalToLocalMap globalToLocalMap; //!< global-to-local mapping
  std::vector< global_int_t > localToGlobalMap; //!< local-to-global mapping
  mutable bool isDotProductOptimized;
  mutable bool isSpmvOptimized;
  mutable bool isMgOptimized;
  mutable bool isWaxpbyOptimized;
  /*!
   This is for storing optimized data structres created in OptimizeProblem and
   used inside optimized ComputeSPMV().
   */
  mutable struct SparseMatrix_STRUCT * Ac; // Coarse grid matrix
  mutable MGData * mgData; // Pointer to the coarse level data for this fine matrix
  void * optimizationData;  // pointer that can be used to store implementation-specific data

#ifndef HPCG_NO_MPI
  local_int_t numberOfExternalValues; //!< number of entries that are external to this process
  int numberOfSendNeighbors; //!< number of neighboring processes that will be send local data
  local_int_t totalToBeSent; //!< total number of entries to be sent
  local_int_t * elementsToSend; //!< elements to send to neighboring processes
  int * neighbors; //!< neighboring processes
  local_int_t * receiveLength; //!< lenghts of messages received from neighboring processes
  local_int_t * sendLength; //!< lenghts of messages sent to neighboring processes
  double * sendBuffer; //!< send buffer for non-blocking sends
#endif
  local_int_t * boundaryRows; //!< rows that contain less than 27 nonzeros
  local_int_t numOfBoundaryRows;
  local_int_t *mtxL;
  global_int_t *mtxG;
  double *mtxA;
  local_int_t nproc;
  local_int_t *work;
  local_int_t *scounts;
  local_int_t *rcounts;
  local_int_t *sdispls;
  local_int_t *rdispls;
};
typedef struct SparseMatrix_STRUCT SparseMatrix;

/*!
  Initializes the known system matrix data structure members to 0.

  @param[in] A the known system matrix
 */
inline void InitializeSparseMatrix(SparseMatrix & A, Geometry * geom) {
  A.title = 0;
  A.geom = geom;
  A.totalNumberOfRows = 0;
  A.totalNumberOfNonzeros = 0;
  A.localNumberOfRows = 0;
  A.localNumberOfColumns = 0;
  A.localNumberOfNonzeros = 0;
  A.nonzerosInRow = 0;
  A.mtxIndG = 0;
  A.mtxIndL = 0;
  A.matrixValues = 0;
  A.matrixDiagonal = 0;
  A.boundaryRows = 0;
  A.numOfBoundaryRows = 0;
  A.nproc = 1;

  // Optimization is ON by default. The code that switches it OFF is in the
  // functions that are meant to be optimized.
  A.isDotProductOptimized = true;
  A.isSpmvOptimized       = true;
  A.isMgOptimized      = true;
  A.isWaxpbyOptimized     = true;

#ifndef HPCG_NO_MPI
  A.numberOfExternalValues = 0;
  A.numberOfSendNeighbors = 0;
  A.totalToBeSent = 0;
  A.elementsToSend = 0;
  A.neighbors = 0;
  A.receiveLength = 0;
  A.sendLength = 0;
  A.sendBuffer = 0;
#endif
  A.mgData = 0; // Fine-to-coarse grid transfer initially not defined.
  A.Ac =0;
  
  A.optimizationData=NULL;
  return;
}

/*!
  Copy values from matrix diagonal into user-provided vector.

  @param[in] A the known system matrix.
  @param[inout] diagonal  Vector of diagonal values (must be allocated before call to this function).
 */
inline void CopyMatrixDiagonal(SparseMatrix & A, Vector & diagonal) {
#ifndef HPCG_LOCAL_LONG_LONG
    SparseMatrix *Ac = &A;
    struct optData *optData = (struct optData *)Ac->optimizationData;
    double * dv = diagonal.values;
    for (local_int_t i=0; i<A.localNumberOfRows; ++i) dv[i] = optData->diag[i];
#else
    double ** curDiagA = A.matrixDiagonal;
    double * dv = diagonal.values;
    assert(A.localNumberOfRows==diagonal.localLength);
    for (local_int_t i=0; i<A.localNumberOfRows; ++i) dv[i] = *(curDiagA[i]);
#endif
  return;
}
/*!
  Replace specified matrix diagonal value.

  @param[inout] A The system matrix.
  @param[in] diagonal  Vector of diagonal values that will replace existing matrix diagonal values.
 */
inline void ReplaceMatrixDiagonal(SparseMatrix & A, Vector & diagonal) {
    double ** curDiagA = A.matrixDiagonal;
    double * dv = diagonal.values;
    assert(A.localNumberOfRows==diagonal.localLength);
    for (local_int_t i=0; i<A.localNumberOfRows; ++i) *(curDiagA[i]) = dv[i];
  return;
}

inline void ReplaceMKLMatrixDiagonal(SparseMatrix & A, Vector & diagonal)
{
    SparseMatrix *Ac = &A;
    struct optData *optData = (struct optData *)Ac->optimizationData;
    sparse_status_t stat = SPARSE_STATUS_SUCCESS;
    sparse_matrix_t csrA = (sparse_matrix_t)optData->csrA;

    for(local_int_t i=0; i<Ac->localNumberOfRows; i++)
    {
        stat = mkl_sparse_d_set_value(csrA, i, i, diagonal.values[i]);
        optData->diag[i] = diagonal.values[i];
    }
}
/*!
  Deallocates the members of the data structure of the known system matrix provided they are not 0.

  @param[in] A the known system matrix
 */
inline void DeleteMatrix(SparseMatrix & A) {

#ifdef HPCG_LOCAL_LONG_LONG
#ifndef HPCG_CONTIGUOUS_ARRAYS
  for (local_int_t i = 0; i< A.localNumberOfRows; ++i) {
    delete [] A.matrixValues[i];
    delete [] A.mtxIndG[i];
    delete [] A.mtxIndL[i];
  }
#else
  delete [] A.matrixValues[0];
  delete [] A.mtxIndG[0];
  delete [] A.mtxIndL[0];
#endif
  if (A.title)                  delete [] A.title;
  if (A.nonzerosInRow)          delete [] A.nonzerosInRow;
  if (A.mtxIndG)                delete [] A.mtxIndG;
  if (A.mtxIndL)                delete [] A.mtxIndL;
  if (A.matrixValues)           delete [] A.matrixValues;
  if (A.matrixDiagonal)         delete [] A.matrixDiagonal;

#ifndef HPCG_NO_MPI
  if (A.elementsToSend)         delete [] A.elementsToSend;
  if (A.neighbors)              delete [] A.neighbors;
  if (A.receiveLength)          delete [] A.receiveLength;
  if (A.sendLength)             delete [] A.sendLength;
  if (A.sendBuffer)             delete [] A.sendBuffer;
#endif

  if (A.geom!=0) { DeleteGeometry(*A.geom); delete A.geom; A.geom = 0;}
  if (A.Ac!=0) { DeleteMatrix(*A.Ac); delete A.Ac; A.Ac = 0;} // Delete coarse matrix
  if (A.mgData!=0) { DeleteMGData(*A.mgData); delete A.mgData; A.mgData = 0;} // Delete MG data

#else
  if (A.title)                  delete [] A.title;
  if (A.nonzerosInRow) { MKL_free(A.nonzerosInRow); A.nonzerosInRow  = NULL; }
  if (A.matrixDiagonal){ MKL_free(A.matrixDiagonal);A.matrixDiagonal = NULL; }
  if (A.boundaryRows)  { MKL_free( A.boundaryRows) ;A.boundaryRows   = NULL; }

#ifndef HPCG_NO_MPI
  MKL_free(A.elementsToSend);
  MKL_free(A.neighbors);
  MKL_free(A.receiveLength);
  MKL_free(A.sendLength);
  MKL_free(A.sendBuffer);
#endif

  struct optData *optData = (struct optData *)A.optimizationData;
  if ( optData != NULL )
  {
      MKL_free(optData->dtmp);
      MKL_free(optData->bmap);
      MKL_free(optData->diag);

      sparse_matrix_t csrA = (sparse_matrix_t)optData->csrA;
      sparse_matrix_t csrB = (sparse_matrix_t)optData->csrB;
      mkl_sparse_destroy(csrA);
      mkl_sparse_destroy(csrB);
      MKL_free(optData);
  }

  if (A.geom!=0) { DeleteGeometry(*A.geom); delete A.geom; A.geom = 0;}
  if (A.Ac!=0) { DeleteMatrix(*A.Ac); delete A.Ac; A.Ac = 0;} // Delete coarse matrix
  if (A.mgData!=0) { DeleteMGData(*A.mgData); delete A.mgData; A.mgData = 0;} // Delete MG data
#endif
  return;
}

inline void init_optData(struct optData optData)
{
    optData.dtmp  = NULL;
    optData.dtmp2 = NULL;
    optData.dtmp3 = NULL;
    optData.dtmp4 = NULL;
    optData.diag  = NULL;
    optData.csrA  = NULL;
    optData.csrB  = NULL;
    optData.bmap  = NULL;
}

#endif // SPARSEMATRIX_HPP
