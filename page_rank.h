//
//  page_rank.h
//  PageRank-MPI
//
//  Created by QiuChusheng on 9/12/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//

#ifndef PageRank_MPI_page_rank_h
#define PageRank_MPI_page_rank_h

#include "pr_graph.h"

#define THRED_SUCCESS 0
#define THRED_FAILURE -1

/*
 page rank algorithm
 @parameters:
    graph: the graph to calculate on.
    nr_nodes: number of nodes in the graph.
    threshold: the threshold used to decide convergence.
    d: the dangling parameter for the page rank formula
    ret: the double array used to store the result.
 */
void calculate_page_rank(inv_graph_t *graph,int nr_nodes, double threshold, double d, double ret[]);
int check_threhold(const double *pr1, const double *pr2, int nr_elem, double thred);
#endif
