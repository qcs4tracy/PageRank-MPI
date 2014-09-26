//
//  main.c
//  PageRank-MPI
//
//  Created by QiuChusheng on 9/10/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//
/* C Example */
#include <stdio.h>
#include <string.h>
#include <mpi.h>

#define MSG_TAG 200
#define NR_BYTES_TAG 201
#define MASTER 0

int main (int argc, char *argv[]){

    int rank, size;
    char proc_name[200];
    int name_len;
    MPI_Status status;
    int m[30], w[10], a[0];
    MPI_Request sreq = MPI_REQUEST_NULL, rreq[2] = {MPI_REQUEST_NULL,MPI_REQUEST_NULL};
    float dangling = 0.15, rec;
    int ct[3] = {0,10,10};
    int disp[3] = {0,0,10};
    
    bzero(proc_name, sizeof(proc_name));
    
    MPI_Init (&argc, &argv);	/* starts MPI */
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* get current process id */
    MPI_Comm_size (MPI_COMM_WORLD, &size);	/* get number of processes */
    //MPI_Get_processor_name(proc_name, &name_len);
    //printf("%s", proc_name);
    
    if (rank == 0) {
        //the master process
//        strcpy(proc_name, "Message from master process.");
//        name_len = (int)strlen(proc_name);
//        MPI_Ibcast(&name_len, 1, MPI_INT, MASTER, MPI_COMM_WORLD, &rreq[0]);
//        MPI_Bcast(proc_name, name_len, MPI_CHAR, MASTER, MPI_COMM_WORLD);
//        //MPI_Send(&name_len, 1, MPI_INT, 1, NR_BYTES_TAG, MPI_COMM_WORLD);
//        //MPI_Send(proc_name, name_len, MPI_CHAR, 1, MSG_TAG, MPI_COMM_WORLD);
//        MPI_Wait(&rreq[0], MPI_STATUS_IGNORE);
//        MPI_Isend(&dangling, 1, MPI_FLOAT, 1, MSG_TAG, MPI_COMM_WORLD, &rreq[0]);
//        MPI_Wait(&rreq[0], &status);
//        printf("master: source:%d tag:%d error:%d \n", status.MPI_SOURCE, status.MPI_TAG, status.MPI_ERROR);
        
        
        MPI_Gatherv(NULL, 0, MPI_INT, m, ct, disp, MPI_INT, MASTER, MPI_COMM_WORLD);
        for (int i = 0; i < 30; i++) {
            printf("%d ", m[i]);
        }
        printf("\n");
    } else {
        
//        //MPI_Recv(&name_len, 1, MPI_INT, MASTER, NR_BYTES_TAG, MPI_COMM_WORLD, &status);
//        MPI_Ibcast(&name_len, 1, MPI_INT, MASTER, MPI_COMM_WORLD, &rreq[0]);
//        MPI_Wait(&rreq[0], &status);
//        printf("worker name_len from master : %d\n", name_len);
//        if (name_len > 0) {
//            MPI_Bcast(proc_name, name_len, MPI_CHAR, MASTER, MPI_COMM_WORLD);
//            printf("Process with rank %d:Receive message [%s] from master\n", rank, proc_name);
//        }
        
//        if (rank == 1) {
//            MPI_Irecv(&rec, 1, MPI_FLOAT, MASTER, MSG_TAG, MPI_COMM_WORLD, &sreq);
//            MPI_Wait(&sreq, &status);
//            printf("rank %d: receive %f\n", rank, rec);
//            printf("worker: source:%d tag:%d error:%d \n", status.MPI_SOURCE, status.MPI_TAG, status.MPI_ERROR);
//        }
        
        for (int i = 0; i < 10; i++) {
            w[i] = 10;
        }
        
        MPI_Gatherv(w, 10, MPI_INT, NULL, NULL, NULL, MPI_INT, MASTER, MPI_COMM_WORLD);
        
    }

    MPI_Finalize();
    return 0;
}
