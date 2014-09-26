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
    char *fp = "/Users/qiuchusheng/Documents/PageRank-MPI/PageRank-MPI/webgraph";//file path
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
    //TODO:1
    int *set = (int *) calloc(nr_node, sizeof(int));
    inv_adj_node_t *jn;
    int sum=0;

    for (int i = 0; i < 2000; i++) {
        jn = graph.array[i].head;
        while (jn != NULL) {
            set[jn->ol_node->pos] = 1;
            jn = jn->next;
        }
    }
    
    for (int i = 0; i < nr_node; i++) {
        sum += set[i];
    }
    
    printf("total of nodes link to: %d\n", sum);
    return 0;
    //TODO:END 1
    calculate_page_rank(&graph, nr_elem(ht), thred, d, pr);
    printf("--------SEQUENTIAL PAGE RANK--------\n");
    for (int i = 0; i < nr_node; i++) {
        printf("The Page Rank value of Node %lu is %f.\n", graph.array[i].node->index, pr[i]);
    }
    
    free(pr);
    fprintf(stderr, "%f", (double)(clock()-time)/CLOCKS_PER_SEC);
    return 0;
}





