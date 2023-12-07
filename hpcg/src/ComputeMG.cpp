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
 @file ComputeMG.cpp

 HPCG routine
 */

#include "ComputeMG.hpp"
#include "ComputeSYMGS.hpp"
#include "ComputeSPMV.hpp"
#include "ComputeRestriction_ref.hpp"
#include "ComputeProlongation_ref.hpp"

#ifndef HPCG_NO_MPI
#include "ExchangeHalo.hpp"
#include <mpi.h>
#include "Geometry.hpp"
#include <cstdlib>
#endif

int ComputeMG_ref(const SparseMatrix & A, const Vector & r, Vector & x);

/*!
  @param[in] A the known system matrix
  @param[in] r the input vector
  @param[inout] x On exit contains the result of the multigrid V-cycle with r as the RHS, x is the approximation to Ax = r.

  @return returns 0 upon success and non-zero otherwise

  @see ComputeMG_ref
*/
int ComputeMG(const SparseMatrix  & A, const Vector & r, Vector & x)
{
#ifdef HPCG_LOCAL_LONG_LONG
    ComputeMG_ref(A,r,x);
#else
    int ierr = 0;

    if (A.mgData!=0) // Go to next coarse level if defined
    {
        const int numberOfPresmootherSteps = A.mgData->numberOfPresmootherSteps;
        const int numberOfPostsmootherSteps = A.mgData->numberOfPostsmootherSteps;

        if ( numberOfPresmootherSteps > 1 )
        {
            sparse_status_t status = SPARSE_STATUS_SUCCESS;
            struct optData *optData = (struct optData *)A.optimizationData;
            struct matrix_descr descr;
            sparse_matrix_t csrA = (sparse_matrix_t)optData->csrA;
            descr.type = SPARSE_MATRIX_TYPE_SYMMETRIC;
            descr.mode = SPARSE_FILL_MODE_FULL;
            descr.diag = SPARSE_DIAG_NON_UNIT;

            if ( mkl_sparse_d_symgs(SPARSE_OPERATION_NON_TRANSPOSE, csrA, descr, 0.0, r.values, x.values) != SPARSE_STATUS_SUCCESS ) ierr ++;

            for ( int i = 1; i < numberOfPresmootherSteps; ++i ) ierr += ComputeSYMGS(A, r, x);
            ierr += ComputeSPMV(A, x, (*A.mgData->Axf));
        } else
        {
            ierr += ComputeSYMGS_MV(A, r, x, (*A.mgData->Axf));
        }

        ierr += ComputeRestriction_ref(A, r);
        ierr += ComputeMG(*A.Ac,*A.mgData->rc, *A.mgData->xc);
        ierr += ComputeProlongation_ref(A, x);

        for ( int i = 0; i < numberOfPostsmootherSteps; ++i ) ierr += ComputeSYMGS(A, r, x);

        if ( ierr != 0 ) return 1;
    } else
    {
        sparse_status_t status = SPARSE_STATUS_SUCCESS;
        struct optData *optData = (struct optData *)A.optimizationData;
        struct matrix_descr descr;
        sparse_matrix_t csrA = (sparse_matrix_t)optData->csrA;
        descr.type = SPARSE_MATRIX_TYPE_SYMMETRIC;
        descr.mode = SPARSE_FILL_MODE_FULL;
        descr.diag = SPARSE_DIAG_NON_UNIT;

        status = mkl_sparse_d_symgs(SPARSE_OPERATION_NON_TRANSPOSE, csrA, descr, 0.0, r.values, x.values);

        if ( status != SPARSE_STATUS_SUCCESS ) return 1;
    }
#endif
    return 0;
}
