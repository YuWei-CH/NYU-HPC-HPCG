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
 @file ComputeSPMV.cpp

 HPCG routine
 */

#include "ComputeSPMV.hpp"
#include "ComputeSPMV_ref.hpp"

#ifndef HPCG_NO_MPI
#include "ExchangeHalo.hpp"

#include <mpi.h>
#include "Geometry.hpp"
#include <cstdlib>
#endif
/*!
  Routine to compute sparse matrix vector product y = Ax where:
  Precondition: First call exchange_externals to get off-processor values of x

  This routine calls the reference SpMV implementation by default, but
  can be replaced by a custom, optimized routine suited for
  the target system.

  @param[in]  A the known system matrix
  @param[in]  x the known vector
  @param[out] y the On exit contains the result: Ax.

  @return returns 0 upon success and non-zero otherwise

  @see ComputeSPMV_ref
*/

int ComputeSPMV( const SparseMatrix & A, Vector & x, Vector & y)
{
#ifdef HPCG_LOCAL_LONG_LONG
    ComputeSPMV_ref(A,x,y);
#else
    sparse_status_t status = SPARSE_STATUS_SUCCESS;
    struct optData *optData = (struct optData *)A.optimizationData;
    struct matrix_descr descr;
    sparse_matrix_t csrA = (sparse_matrix_t)optData->csrA;
    sparse_matrix_t csrB = (sparse_matrix_t)optData->csrB;
    descr.type = SPARSE_MATRIX_TYPE_SYMMETRIC;
    descr.mode = SPARSE_FILL_MODE_FULL;
    descr.diag = SPARSE_DIAG_NON_UNIT;

    #ifndef HPCG_NO_MPI
    ExchangeHalo(A,x);
    #endif

    status = mkl_sparse_d_mv ( SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrA, descr, x.values, 0.0, y.values );

    if ( A.geom->size > 1 )
    {
        descr.type = SPARSE_MATRIX_TYPE_GENERAL;
        status = mkl_sparse_d_mv ( SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrB, descr, x.values, 0.0, optData->dtmp );
        #ifndef HPCG_NO_OPENMP
        #pragma omp parallel for
        #endif
        #pragma ivdep
        for (local_int_t i=0; i<optData->nrow_b; i++)
        {
            const MKL_INT ind = optData->bmap[i];
            y.values[ind] += optData->dtmp[i];
        }
    }
#endif
    return 0;
}

int ComputeSPMV_DOT( const SparseMatrix & A, Vector & x, Vector & y, double & pAp)
{
    sparse_status_t status = SPARSE_STATUS_SUCCESS;
    struct optData *optData = (struct optData *)A.optimizationData;
    struct matrix_descr descr;
    sparse_matrix_t csrA = (sparse_matrix_t)optData->csrA;
    sparse_matrix_t csrB = (sparse_matrix_t)optData->csrB;
    descr.type = SPARSE_MATRIX_TYPE_SYMMETRIC;
    descr.mode = SPARSE_FILL_MODE_FULL;
    descr.diag = SPARSE_DIAG_NON_UNIT;

    #ifndef HPCG_NO_MPI
    ExchangeHalo(A,x);
    #endif

    status = mkl_sparse_d_dotmv ( SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrA, descr, x.values, 0.0, y.values, &pAp );

    if ( A.geom->size > 1 )
    {
        descr.type = SPARSE_MATRIX_TYPE_GENERAL;
        status = mkl_sparse_d_mv ( SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrB, descr, x.values, 0.0, optData->dtmp );
        double pAp_loc = 0, pAp_red = 0;
        #ifndef HPCG_NO_OPENMP
        #pragma omp parallel for reduction(+:pAp_loc,pAp_red)
        #endif
        #pragma ivdep
        for (local_int_t i=0; i<optData->nrow_b; i++)
        {
            const MKL_INT ind = optData->bmap[i];
            const double x_cur = x.values[ind];
            pAp_red += y.values[ind]*x_cur;
            y.values[ind] += optData->dtmp[i];
            pAp_loc += y.values[ind]*x_cur;
        }
        pAp += (pAp_loc - pAp_red);
    }

    return 0;
}
