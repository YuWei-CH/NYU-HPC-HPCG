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
 @file CG.cpp

 HPCG routine
 */

#include <fstream>

#include <cmath>
#include <cfloat>

#include "hpcg.hpp"

#include "CG.hpp"
#include "CG_ref.hpp"
#include "mytimer.hpp"
#include "ComputeSPMV.hpp"
#include "ComputeSPMV_ref.hpp"
#include "ComputeMG.hpp"
#include "ComputeDotProduct.hpp"
#include "ComputeWAXPBY.hpp"

#ifndef HPCG_NO_MPI
#include "ExchangeHalo.hpp"
#endif

// Use TICK and TOCK to time a code section in MATLAB-like fashion
#define TICK()  t0 = mytimer() //!< record current time in 't0'
#define TOCK(t) t += mytimer() - t0 //!< store time difference in 't' using time in 't0'

/*!
  Routine to compute an approximate solution to Ax = b

  @param[in]    geom The description of the problem's geometry.
  @param[inout] A    The known system matrix
  @param[inout] data The data structure with all necessary CG vectors preallocated
  @param[in]    b    The known right hand side vector
  @param[inout] x    On entry: the initial guess; on exit: the new approximate solution
  @param[in]    max_iter  The maximum number of iterations to perform, even if tolerance is not met.
  @param[in]    tolerance The stopping criterion to assert convergence: if norm of residual is <= to tolerance.
  @param[out]   niters    The number of iterations actually performed.
  @param[out]   normr     The 2-norm of the residual vector after the last iteration.
  @param[out]   normr0    The 2-norm of the residual vector before the first iteration.
  @param[out]   times     The 7-element vector of the timing information accumulated during all of the iterations.
  @param[in]    doPreconditioning The flag to indicate whether the preconditioner should be invoked at each iteration.

  @return Returns zero on success and a non-zero value otherwise.

  @see CG_ref()
*/
int CG(const SparseMatrix & A, CGData & data, const Vector & b, Vector & x,
    const int max_iter, const double tolerance, int & niters, double & normr, double & normr0,
    double * times, bool doPreconditioning) {
#ifdef HPCG_LOCAL_LONG_LONG
    return CG_ref(A, data, b, x, max_iter, tolerance, niters, normr, normr0, times, doPreconditioning);
#else
    double rtz = 0.0, oldrtz = 0.0, alpha = 0.0, beta = 0.0, pAp = 0.0, ff = 0.0;

    double t0 = 0.0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0, t5 = 0.0;
    double t_begin = mytimer();  // Start timing right away

#ifndef HPCG_NO_MPI
    MPI_Request *request = new MPI_Request();
#endif
    normr = 0.0;
    local_int_t nthr = A.nproc;
    local_int_t nrow = A.localNumberOfRows;
    Vector & r = data.r; // Residual vector
    Vector & z = data.z; // Preconditioned residual vector
    Vector & p = data.p; // Direction vector (in MPI mode ncol>=nrow)
    Vector & Ap = data.Ap;

    if (!doPreconditioning && A.geom->rank==0) HPCG_fout << "WARNING: PERFORMING UNPRECONDITIONED ITERATIONS OPT" << std::endl;

#ifdef HPCG_DEBUG
    int print_freq = 1;
    if (print_freq>50) print_freq=50;
    if (print_freq<1)  print_freq=1;
#endif
  // p is of length ncols, copy x to p for sparse MV operation

    double normr_tmp = 0.0;

    TICK();
    #ifndef HPCG_NO_OPENMP
    #pragma omp parallel for num_threads(nthr) reduction(+:normr_tmp)
    #endif
    for ( local_int_t i = 0; i < nrow; ++i )
    {
        r.values[i] = b.values[i];
        normr_tmp += r.values [i] * r.values [i];
    }
    TOCK(t2);
    TICK();
#ifndef HPCG_NO_MPI
    double global_result = 0.0;
    MPI_Allreduce(&normr_tmp, &global_result, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    normr_tmp = global_result;
#endif
    normr = sqrt(normr_tmp);
    // Record initial residual for convergence testing
    normr0 = normr;
    // Convergence check accepts an error of no more than 6 significant digits of tolerance
    ff = normr/normr0-tolerance*(1.0 + 1e-6); //  if (normr/normr0 >= DBL_EPSILON + tolerance + tolerance*1e-6)  no convergence yet
    TOCK(t4);

    int converge_flag = 0;
    if ( ff <= 0.0 )
    {
        converge_flag = 1;
    }

#ifdef HPCG_DEBUG
    if (A.geom->rank==0) HPCG_fout << "Initial Residual = "<< normr << std::endl;
#endif
    // Start iterations

  for (int k=1; (k<=max_iter && ff >= DBL_EPSILON) || (converge_flag == 1 && k <= 50); k++ )
    {
        if (doPreconditioning)
        {
            TICK(); ComputeMG(A, r, z); TOCK(t5); // Apply preconditioner
        } else
        {
            TICK(); CopyVector (r, z); TOCK(t5); // copy r to z (no preconditioning)
        }

        if (k == 1) {
            TICK();
            normr_tmp = 0.0;
            #ifndef HPCG_NO_OPENMP
            #pragma omp parallel for reduction(+:normr_tmp)
            #endif
            for ( local_int_t i = 0; i < nrow; i++ )
            {
                p.values[i] = z.values[i];
                normr_tmp += r.values[i]*z.values[i];
            }
            TOCK(t2);
            TICK();
#ifndef HPCG_NO_MPI
            global_result = 0.0;
            MPI_Allreduce(&normr_tmp, &global_result, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
            rtz = global_result;
#else
            rtz = normr_tmp;
#endif
            TOCK(t4);
        } else {
            TICK();
            oldrtz = rtz;
            normr_tmp = 0.0;
            #ifndef HPCG_NO_OPENMP
            #pragma omp parallel for reduction(+:normr_tmp)
            #endif
            for ( local_int_t i = 0; i < nrow; i++ )
                normr_tmp += r.values[i]*z.values[i];
            TOCK(t1);
            TICK();
#ifndef HPCG_NO_MPI
            global_result = 0.0;
            MPI_Allreduce(&normr_tmp, &global_result, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
            rtz = global_result;
#else
            rtz = normr_tmp;
#endif
            beta = rtz/oldrtz;
            TOCK(t4);
            TICK();
            #ifndef HPCG_NO_OPENMP
            #pragma omp parallel for
            #endif
            for ( local_int_t i = 0; i < nrow; i++ )
            {
                p.values[i] = beta*p.values[i] + z.values[i];
            }
            TOCK(t2);
        }

        if ( A.geom->size > 1 )
        {
            normr_tmp = 0.0;
            TICK(); ComputeSPMV_DOT(A, p, Ap, normr_tmp); TOCK(t3); // Ap = A*p
            TICK();
#ifndef HPCG_NO_MPI
            global_result = 0.0;
            MPI_Allreduce(&normr_tmp, &global_result, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
            pAp = global_result;
#else
            pAp = normr_tmp;
#endif
            TOCK(t4);
        } else
        {
            TICK();
            sparse_status_t status = SPARSE_STATUS_SUCCESS;
            struct optData *optData = (struct optData *)A.optimizationData;
            struct matrix_descr descr;
            sparse_matrix_t csrA = (sparse_matrix_t)optData->csrA;
            descr.type = SPARSE_MATRIX_TYPE_SYMMETRIC;
            descr.mode = SPARSE_FILL_MODE_FULL;
            descr.diag = SPARSE_DIAG_NON_UNIT;

            status = mkl_sparse_d_dotmv ( SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrA, descr, p.values, 0.0, Ap.values, &pAp );
            TOCK(t3);
        }

        TICK();
        alpha = rtz/pAp;
        normr_tmp = 0.0;
        #ifndef HPCG_NO_OPENMP
        #pragma omp parallel for reduction(+:normr_tmp)
        #endif
        for ( local_int_t i = 0; i < nrow; i++ )
        {
            r.values[i] -= alpha * Ap.values[i];
            normr_tmp += r.values[i]*r.values[i];
        }
        TOCK(t2);// r = r - alpha*Ap

        TICK();
#ifndef HPCG_NO_MPI
        global_result = 0.0;
        MPI_Iallreduce(&normr_tmp, &global_result, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD, (MPI_Request *)request);
#endif

        #ifndef HPCG_NO_OPENMP
        #pragma omp parallel for
        #endif
        for ( local_int_t i = 0; i < nrow; i++ )
        {
            x.values[i] += alpha *  p.values[i];
        }
#ifndef HPCG_NO_MPI
        MPI_Wait((MPI_Request *)request, MPI_STATUS_IGNORE);
        normr_tmp = global_result;
#endif

        normr = sqrt(normr_tmp);
        ff = normr/normr0-tolerance;
        niters = k;
        TOCK(t4);
#ifdef HPCG_DEBUG
        if (A.geom->rank==0 && (k%print_freq == 0 || k == max_iter))
            HPCG_fout << "Iteration = "<< k <<" " << tolerance <<" " << A.geom->rank << "   Scaled Residual = "<< normr/normr0 << std::endl;
#endif
    }

  // Store times
    times[0] += mytimer() - t_begin;  // Total time. All done...
    times[1] += t1; // dot-product time
    times[2] += t2; // WAXPBY time
    times[3] += t3; // SPMV time
    times[4] += t4; // AllReduce time
    times[5] += t5; // preconditioner apply time

#ifndef HPCG_NO_MPI
    delete (MPI_Request *)request;
#endif
#endif
    return 0;
}
