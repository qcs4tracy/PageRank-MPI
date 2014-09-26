//
//  page_rank_seq.c
//  PageRank-MPI
//
//  Created by QiuChusheng on 9/14/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pr_graph.h"
#include "page_rank.h"
#include "args_parse.h"

#define N 20000

int main(int argc, char *argv[])
{
    
    double thred = DEF_THRED, d = dangling;//threshold and dangling, initialize to default value
    char *fp = "webgraph";//file path
    /*set up the graph*/
    double *pr;
    int nr_node;
    inv_graph_t graph;
    hash_table_t *ht;
    
    clock_t time = clock();
    
    parse_args(argc, argv, &thred, &d, &fp);
    
    ht = create_ol_node_table(N, NULL);
    init_graph(&graph, N, NULL);
    
    if (setup_graph_from_file(ht, &graph, fp) < 0) exit(EXIT_FAILURE);
    
    
    nr_node = nr_elem(ht);
    
    pr = (double *)calloc(nr_node, sizeof(double));
    
    calculate_page_rank(&graph, nr_elem(ht), thred, d, pr);
    printf("--------SEQUENTIAL PAGE RANK--------\n");
    for (int i = 0; i < nr_node; i++) {
        printf("The Page Rank value of Node %lu is %f.\n", graph.array[i].node->index, pr[i]);
    }
    
    free(pr);
    fprintf(stderr, "%f", (double)(clock()-time)/CLOCKS_PER_SEC);
    return 0;
}





