//
//  page_rank_mpi.c
//  PageRank-MPI
//
//  Created by QiuChusheng on 9/12/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//

#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include "mpi_pr.h"
#include "args_parse.h"


int main(int argc, char *argv[])
{
    int nr_procs, proc_rank;
    double thred = DEF_THRED, d = dangling;//threshold and dangling, initialize to default value
    char *fp = "webgraph";//file path
    
    clock_t time;
    
    /*initialize the environment*/
    MPI_Init(&argc, &argv);
    /*get the size of the communicator*/
    MPI_Comm_size(MPI_COMM_WORLD, &nr_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
    
    parse_args(argc, argv, &thred, &d, &fp);
    
    /*if the process is the master*/
    if (proc_rank == MASTER) {
        time = clock();
        run_master(thred, fp);
        fprintf(stderr, "%f", (double)(clock()-time)/CLOCKS_PER_SEC);
    } else {/*otherwise*/
        
        run_worker(d);
    }
    
    MPI_Finalize();
    
    return 0;
}





