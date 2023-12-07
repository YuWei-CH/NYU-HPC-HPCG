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

#ifndef HPCG_MPI_API_HPP
#define HPCG_MPI_API_HPP

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int HPCG_MPI_Comm;
typedef int HPCG_MPI_Datatype;
typedef int HPCG_MPI_Op;

/* Enough bits to hold a pointer */
typedef uintptr_t HPCG_MPI_Request;

/* BUGBUGBUG: The fields below are never marshalled */
typedef struct HPCG_MPI_Status {
	int count;
	int cancelled;
	int HPCG_MPI_SOURCE;
	int HPCG_MPI_TAG;
	int HPCG_MPI_ERROR;
	char filler[64];
} HPCG_MPI_Status;

/* (Some) MPI constants */
extern int HPCG_MPI_ANY_SOURCE;

extern int HPCG_MPI_COMM_NULL;
extern int HPCG_MPI_COMM_WORLD;
extern int HPCG_MPI_MAX_PROCESSOR_NAME;

extern void* HPCG_MPI_IN_PLACE;

extern int HPCG_MPI_CHAR;
extern int HPCG_MPI_BYTE;
extern int HPCG_MPI_DOUBLE;
extern int HPCG_MPI_DOUBLE_INT;
extern int HPCG_MPI_DOUBLE_PRECISION;
extern int HPCG_MPI_FLOAT;
extern int HPCG_MPI_LONG_LONG_INT;
extern int HPCG_MPI_SCALAPACK_INT;

extern int HPCG_MPI_MAXLOC;
extern int HPCG_MPI_MAX;
extern int HPCG_MPI_MIN;
extern int HPCG_MPI_SUM;

extern HPCG_MPI_Request HPCG_MPI_REQUEST_NULL;
extern HPCG_MPI_Status* HPCG_MPI_STATUSES_IGNORE;
extern HPCG_MPI_Status* HPCG_MPI_STATUS_IGNORE;

extern int HPCG_MPI_SIMILAR;
extern int HPCG_MPI_SUCCESS;
extern int HPCG_MPI_UNDEFINED;

/* (Some) MPI functions */
int HPCG_MPI_Initmpi(int *argc, char ***argv);
int HPCG_MPI_Finalize();
int HPCG_MPI_Get_processor_name(char *name, int *resultlen);
double HPCG_MPI_Wtime();

int HPCG_MPI_Comm_rank(HPCG_MPI_Comm comm, int *rank);
int HPCG_MPI_Comm_size(HPCG_MPI_Comm comm, int *size);
int HPCG_MPI_Request_free(HPCG_MPI_Request *request);

int HPCG_MPI_Send(void *buf, int count, HPCG_MPI_Datatype datatype,
		int dest, int tag, HPCG_MPI_Comm comm);
int HPCG_MPI_Irecv(void *buf, int count, HPCG_MPI_Datatype datatype,
		int source, int tag, HPCG_MPI_Comm comm, HPCG_MPI_Request *request);
int HPCG_MPI_Bcast(void *buffer, int count, HPCG_MPI_Datatype datatype,
		int root, HPCG_MPI_Comm comm);

int HPCG_MPI_Wait(HPCG_MPI_Request *request, HPCG_MPI_Status *status);
int HPCG_MPI_Barrier(HPCG_MPI_Comm comm);
int HPCG_MPI_Waitall(int count,
		HPCG_MPI_Request *requests, HPCG_MPI_Status *statuses);

int HPCG_MPI_Allreduce(void *sendbuf, void *recvbuf, int count,
		HPCG_MPI_Datatype datatype, HPCG_MPI_Op op, HPCG_MPI_Comm comm);
int HPCG_MPI_Allgather(void *sendbuf, int sendcount, HPCG_MPI_Datatype sendtype,
		void *recvbuf, int recvcount, HPCG_MPI_Datatype recvtype,
		HPCG_MPI_Comm comm);

int HPCG_MPI_Recv_init(void *buf, int count, HPCG_MPI_Datatype datatype,
		int source ,int tag, HPCG_MPI_Comm comm, HPCG_MPI_Request *request);
int HPCG_MPI_Send_init(void *buf, int count, HPCG_MPI_Datatype datatype,
		int dest, int tag, HPCG_MPI_Comm comm, HPCG_MPI_Request *request);
int HPCG_MPI_Startall(int a, HPCG_MPI_Request *requests);

int HPCG_MPI_Error_string(int errorcode, char *string, int *resultlen);

int HPCG_MPI_Gather(void *sendbuf, int sendcount, HPCG_MPI_Datatype sendtype,
        void *recvbuf, int recvcount, HPCG_MPI_Datatype recvtype,
        int root, HPCG_MPI_Comm comm);

int HPCG_MPI_Test(HPCG_MPI_Request *request, int *flag, HPCG_MPI_Status *status);

int HPCG_MPI_Cancel(HPCG_MPI_Request *request);

int HPCG_MPI_Abort(HPCG_MPI_Comm comm, int errorcode);

int HPCG_MPI_Testany(int count, HPCG_MPI_Request array_of_requests[], int *indx,
               int *flag, HPCG_MPI_Status *status);

int HPCG_MPI_Iallreduce(void *sendbuf, void *recvbuf, int count,
        HPCG_MPI_Datatype datatype, HPCG_MPI_Op op, HPCG_MPI_Comm comm,
        HPCG_MPI_Request *hpcg_mpi_request);

#ifdef __cplusplus
}
#endif
#endif // HPCG_MPI_API_HPP

