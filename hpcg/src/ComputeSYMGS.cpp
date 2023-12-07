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
 @file ComputeSYMGS.cpp

 HPCG routine
 */

#include "ComputeSYMGS.hpp"
#include "ComputeSYMGS_ref.hpp"
#ifndef HPCG_NO_OPENMP
#include <omp.h>
#endif

#ifndef HPCG_NO_MPI
#include "ExchangeHalo.hpp"
#include <mpi.h>
#include "Geometry.hpp"
#include <cstdlib>
#endif

#include "mkl.h"
/*!
  Routine to compute one step of symmetric Gauss-Seidel:

  Assumption about the structure of matrix A:
  - Each row 'i' of the matrix has nonzero diagonal value whose address is matrixDiagonal[i]
  - Entries in row 'i' are ordered such that:
       - lower triangular terms are stored before the diagonal element.
       - upper triangular terms are stored after the diagonal element.
       - No other assumptions are made about entry ordering.

  Symmetric Gauss-Seidel notes:
  - We use the input vector x as the RHS and start with an initial guess for y of all zeros.
  - We perform one forward sweep.  Since y is initially zero we can ignore the upper triangular terms of A.
  - We then perform one back sweep.
       - For simplicity we include the diagonal contribution in the for-j loop, then correct the sum after

  @param[in]  A the known system matrix
  @param[in]  r the input vector
  @param[inout] x On entry, x should contain relevant values, on exit x contains the result of one symmetric GS sweep with r as the RHS.

  @return returns 0 upon success and non-zero otherwise

  @warning Early versions of this kernel (Version 1.1 and earlier) had the r and x arguments in reverse order, and out of sync with other kernels.

  @see ComputeSYMGS_ref
*/

int ComputeSYMGS( const SparseMatrix & A, const Vector & r, Vector & x)
{
    sparse_status_t status = SPARSE_STATUS_SUCCESS;
    struct optData *optData = (struct optData *)A.optimizationData;
    struct matrix_descr descr;
    sparse_matrix_t csrA = (sparse_matrix_t)optData->csrA;
    sparse_matrix_t csrB = (sparse_matrix_t)optData->csrB;

    descr.type = SPARSE_MATRIX_TYPE_TRIANGULAR;
    descr.mode = SPARSE_FILL_MODE_UPPER;
    descr.diag = SPARSE_DIAG_NON_UNIT;

    if(A.geom->size > 1)
    {
        #ifndef HPCG_NO_MPI
        ExchangeHalo(A,x);
        #endif

        status = mkl_sparse_d_mv(SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrA, descr, x.values, 0.0, optData->dtmp4);

        descr.type = SPARSE_MATRIX_TYPE_GENERAL;
        descr.mode = SPARSE_FILL_MODE_FULL;

        status = mkl_sparse_d_mv(SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrB, descr, x.values, 0.0, optData->dtmp2);

        #ifndef HPCG_NO_OPENMP
        #pragma omp parallel for
        #endif
        for ( local_int_t i = 0; i < A.localNumberOfRows; i ++ )
            optData->dtmp[i] = r.values[i] - optData->dtmp4[i];

        #ifndef HPCG_NO_OPENMP
        #pragma omp parallel for
        #endif
        #pragma ivdep
        for (local_int_t i=0; i < optData->nrow_b; i++) optData->dtmp[optData->bmap[i]] -= optData->dtmp2[i];

        descr.type = SPARSE_MATRIX_TYPE_SYMMETRIC;
        descr.mode = SPARSE_FILL_MODE_LOWER;

        mkl_sparse_d_trsv ( SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrA, descr, optData->dtmp, optData->dtmp3);

        #ifndef HPCG_NO_OPENMP
        #pragma omp parallel for
        #endif
        for ( local_int_t i = 0; i < A.localNumberOfRows; i ++ )
            optData->dtmp3[i] = optData->dtmp3[i]*optData->diag[i] + optData->dtmp4[i];

        descr.mode = SPARSE_FILL_MODE_UPPER;
        mkl_sparse_d_trsv ( SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrA, descr, optData->dtmp3, x.values);
    } else
    {
        descr.type = SPARSE_MATRIX_TYPE_SYMMETRIC;
        descr.mode = SPARSE_FILL_MODE_FULL;
        status = mkl_sparse_d_symgs(SPARSE_OPERATION_NON_TRANSPOSE, csrA, descr, 1.0, r.values, x.values);
    }
    return 0;
}

int ComputeSYMGS_MV( const SparseMatrix & A, const Vector & r, Vector & x, Vector & y )
{
    sparse_status_t status = SPARSE_STATUS_SUCCESS;
    struct optData *optData = (struct optData *)A.optimizationData;
    struct matrix_descr descr;
    sparse_matrix_t csrA = (sparse_matrix_t)optData->csrA;
    sparse_matrix_t csrB = (sparse_matrix_t)optData->csrB;

    descr.type = SPARSE_MATRIX_TYPE_SYMMETRIC;
    descr.mode = SPARSE_FILL_MODE_LOWER;
    descr.diag = SPARSE_DIAG_NON_UNIT;

    if(A.geom->size > 1)
    {
        mkl_sparse_d_trsv ( SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrA, descr, r.values, optData->dtmp3);
        #ifndef HPCG_NO_OPENMP
        #pragma omp parallel for
        #endif
        for ( local_int_t i = 0; i < A.localNumberOfRows; i ++ ) optData->dtmp3[i] *= optData->diag[i];

        descr.mode = SPARSE_FILL_MODE_UPPER;
        mkl_sparse_d_trsv ( SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrA, descr, optData->dtmp3, x.values);

        descr.type = SPARSE_MATRIX_TYPE_TRIANGULAR;
        descr.mode = SPARSE_FILL_MODE_LOWER;

        #ifndef HPCG_NO_OPENMP
        #pragma omp parallel for
        #endif
        for ( local_int_t i = 0; i < A.localNumberOfRows; i ++ )
            y.values[i] = optData->dtmp3[i] - x.values[i]*optData->diag[i];

        status = mkl_sparse_d_mv(SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrA, descr, x.values, 1.0, y.values);

        #ifndef HPCG_NO_MPI
        ExchangeHalo(A,x);
        #endif

        descr.type = SPARSE_MATRIX_TYPE_GENERAL;
        descr.mode = SPARSE_FILL_MODE_FULL;
        status = mkl_sparse_d_mv(SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrB, descr, x.values, 0.0, optData->dtmp3);
        #ifndef HPCG_NO_OPENMP
        #pragma omp parallel for
        #endif
        #pragma ivdep
        for (local_int_t i=0; i<optData->nrow_b; i++) y.values[optData->bmap[i]] += optData->dtmp3[i];
    } else
    {
        descr.mode = SPARSE_FILL_MODE_FULL;
        status = mkl_sparse_d_symgs_mv(SPARSE_OPERATION_NON_TRANSPOSE, csrA, descr, 0.0, r.values, x.values, y.values);
    }
    return 0;
}
