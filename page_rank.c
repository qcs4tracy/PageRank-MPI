//
//  page_rank.c
//  PageRank-MPI
//
//  Created by QiuChusheng on 9/12/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//

#include <stdio.h>
#include "page_rank.h"


/*used to check whether the result converge or not*/
int check_threhold(const double *pr1, const double *pr2, int nr_elem, double thred)
{
    for (int i = 0; i < nr_elem; i++) {
        if ((pr1[i] - pr2[i]) > thred) {
            return THRED_FAILURE;
        }
    }
    return THRED_SUCCESS;
}

/*implemetation of the page rank algorithm*/
void calculate_page_rank(inv_graph_t *graph,int nr_nodes, double threshold, double d, double ret[])
{
    /* - pr and pre_pr is used to store two iterations of result
       - t is the 1 minus dangling parameter
       - nr_e denotes the total # of nodes in the graph
     */
    double pr[nr_nodes],pre_pr[nr_nodes];
    double t = 1 - d;
    double *pPr, *pPrevPr, *swp;
    int nr_e = nr_nodes;
    
    /*temp variable used to calculate page rank values*/
    inv_adj_node_t *tmp;
    ol_node_t ol_tmp;
    
    
    d = d/nr_e;
    
    
    /*initialize the pr array*/
    for (int j = 0; j < nr_e; j++) {
        pre_pr[j] = ((double)1)/nr_e;
    }
    
    /*the pre_pr array is initialized and treated as the result of last iteration*/
    pPrevPr = pre_pr;
    /*result of the next iteration will be stored in array pPr pointed to*/
    pPr = pr;
    
    /* pr[i] = 0.15/N `d/nr_e` + 0.85*sum{(pr[j];j->i)/out(j)} */
    for (; ; ) {
        for (int i = 0; i < nr_e; i++) {
            tmp = graph->array[i].head;
            pPr[i] = 0;
            while (tmp != NULL) {
                ol_tmp = tmp->ol_node;
                /*the sum{(pr[j];j->i)/out(j)} part of the formula*/
                pPr[i] += ( pPrevPr[ol_tmp->pos] / ol_tmp->nr_outlinks );
                tmp = tmp->next;
            }
            /* the final value */
            pPr[i] = d + pPr[i]*t;
        }
        
        if (check_threhold(pPr, pPrevPr, nr_e, threshold) == THRED_SUCCESS) {
            /*array pPr pointing to is the result*/
            break;
        }
        /*swap two array*/
        swp = pPrevPr;
        pPrevPr = pPr;
        pPr = swp;
    }
    
    /*copy the result to ret array*/
    for (int i = 0; i < nr_e; i++) {
        ret[i] = pPr[i];
    }
}


