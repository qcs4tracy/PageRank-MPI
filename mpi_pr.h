//
//  mpi_pr.h
//  PageRank-MPI
//
//  Created by QiuChusheng on 9/13/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//

#ifndef PageRank_MPI_mpi_pr_h
#define PageRank_MPI_mpi_pr_h

#define MASTER 0

void run_master(double thred, const char* fp);
void run_worker(double dangl);

#endif
