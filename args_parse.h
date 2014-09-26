//
//  args_parse.h
//  PageRank-MPI
//
//  Created by QiuChusheng on 9/14/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//

#ifndef PageRank_MPI_args_parse_h
#define PageRank_MPI_args_parse_h

//default dangling parameter
#define dangling 0.15
//default threshold used to decide convergence
#define DEF_THRED 0.01

/*function used to parse the comman line arguments*/
void parse_args(int argc, char *argv[], double *threshold, double *d, char **pfn);

#endif
