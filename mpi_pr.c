//
//  mpi_pr.c
//  PageRank-MPI
//
//  Created by QiuChusheng on 9/13/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//

#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include "page_rank.h"
#include "pr_graph.h"
#include "mpi_pr.h"
#include "palloc.h"


#define N 25000

#define PR_TAG 100
#define SIZE_TAG 102
#define OUTLK_TAG 101
#define IN_DEG_TAG 103
#define CTL_TAG 200

#define CTL_CONT 1
#define CTL_STOP 2

/*set up info of nodes pointing to nodes assigned to a worker's 
  eg. each node assigned to worker rank 1 has a set of other nodes link to it
     in the web graph. The function encapsulate all dependent nodes needed by
      each worker and send to it.
 */
void setup_dependencies_info(inv_graph_t *g, int nr_prc, int nr_nodes);

/*calculate counts and displacements array used by MPI_Gatherv function*/
void calc_per_worker_jcounts_displ(int nr_prc, int nr_nodes, int counts[], int displ[]);

/*page rank algorithm called by MPI worker*/
void worker_cal_PR(int n_size, int nr_node, double d,
                   const int in_node_arr[], int arr_size,
                   const int outlinks[], const double pr[], double local_pr[]);






/*get the interval of nodes the assigned to woker identified by rank*/
void get_job_interval(int *beg, int *end, int rank, int nn, int np)
{
    /*exculding the master*/
    np = np - 1;
    int per_size = nn/np;
    int remain = nn % np;
    /*if nn < np */
    if (per_size == 0) {
        
        *beg = *end = rank;
        if (rank > nn) { *beg = *end = 0; }
        return;
    }
    
    if (remain == 0) {
        *beg = (rank-1)*per_size + 1;
        *end = *beg + per_size - 1;
        return;
    }
    
    /*remainder is not 0*/
    if (rank <= remain) {
        *beg = (rank - 1)*(per_size + 1) + 1;
        *end = *beg + per_size;
        return;
    }
    
    *beg = remain*(per_size + 1) + (rank - remain - 1)*per_size + 1;
    *end = *beg + per_size - 1;
}

/*
 process with rank 0 will CALL the function.
*/
void run_master(double thred, const char* fp)
{
    
    int nr_node, nr_proc;
    MPI_Request sreq[2];
    int *outlink;
    double *pr, *pPrev, *tmp;
    int *jcount, *displ; /*used by MPI_Gatherv*/
    int ctl_flag = CTL_CONT;
    int i;
    
    /*create a memory pool with each block in the pool having a size of 8KB (2 pages)*/
    mem_pool_t *pool = mem_create_pool(MEM_POOL_DEFAUT_SIZE);
    
    /*set up the graph*/
    inv_graph_t graph;
    hash_table_t *ht = create_ol_node_table(N, pool);
    init_graph(&graph, N, pool);
    
    if (setup_graph_from_file(ht, &graph, fp) < 0) return;
    
    /*------------------------------START OF SETTING UP THE COMPUTE ENVIRONMENT---------------------*/
    
    /*bcast total # of node in the graph*/
    nr_node = nr_elem(ht);
    MPI_Ibcast(&nr_node, 1, MPI_INT, MASTER, MPI_COMM_WORLD, &sreq[0]);
    
    MPI_Comm_size(MPI_COMM_WORLD, &nr_proc);
    
    //allocate the memory used to store all info needed
    jcount = (int *)mem_palloc(pool, sizeof(int)*2*nr_proc + (sizeof(int) + sizeof(double)*2)*nr_node);
    displ = jcount + nr_proc;
    outlink = jcount + nr_proc;
    pr = (double *)((int *)outlink + nr_node);
    pPrev = pr + nr_node;
    
    /*initialize page rank values and outlinks for each node*/
    for (i = 0; i < nr_node; i++)
    {
        pr[i] = ((double)1/nr_node);
        outlink[i] = graph.array[i].node->nr_outlinks;
    }
    
    /*broadcast outlink array to all workers*/
    MPI_Ibcast(outlink, nr_node, MPI_INT, MASTER, MPI_COMM_WORLD, &sreq[1]);
    
    setup_dependencies_info(&graph, nr_proc, nr_node);
    calc_per_worker_jcounts_displ(nr_proc, nr_node, jcount, displ);
 
    MPI_Waitall(2, sreq, MPI_STATUSES_IGNORE);
    /*------------------------------END OF SETTING UP THE COMPUTE ENVIRONMENT-----------------------*/
    
    
    while (1) {
        
        MPI_Bcast(pr, nr_node, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
        
        /*gather data from all workers*/
        MPI_Gatherv(NULL, 0, MPI_DOUBLE, pPrev, jcount, displ, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
        
        /*the pPrev array now contain the newly computed page rank value*/
        tmp = pr;
        pr = pPrev;
        pPrev = tmp;
        
        /*converge, stop*/
        if (check_threhold(pr, pPrev, nr_node, thred) == THRED_SUCCESS) {
            ctl_flag = CTL_STOP;
            MPI_Bcast(&ctl_flag, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
            break;
        }
        /*continue*/
        MPI_Bcast(&ctl_flag, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
        
    }
    
    printf("finish..\n");
    //TODO: process pr array which contain page rank values
    for (int ctn = 0; ctn < nr_node; ctn++) {
        printf("PageRank value of Node %lu: %f\n", graph.array[ctn].node->index, pr[ctn]);
    }
    
    mem_destroy_pool(pool);
  
}


/* worker processes will CALL this function*/
void run_worker(double dangl)
{
    int nr_node, rank, n_procs, n_size;
    int begin = 0, end = 0;
    MPI_Request rreq[2];
    double *pr, *local_pr;
    int *outlink;
    int in_node_arr_size;
    int *in_node_arr;
    int ctl_flag;
    
    /*------------------------------START OF SETTING UP THE COMPUTE ENVIRONMENT---------------------*/
    
    //get the size of processes in the communicator
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
    //the worker's rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    /*participate in the broadcast operation and get the total # of nodes in the graph*/
    MPI_Ibcast(&nr_node, 1, MPI_INT, MASTER, MPI_COMM_WORLD, &rreq[0]);
    
    /*wait for request to complete, cause need # of nodes as parameter*/
    MPI_Wait(&rreq[0], MPI_STATUS_IGNORE);
    
    get_job_interval(&begin, &end, rank, nr_node, n_procs);
    
    if (begin == 0 && end == 0) {
        return;
    }
    
    n_size = end - begin + 1;
    //align with the index in the array, starting with 0 instead of 1.
    begin--;
    end--;
    
    //allocate the memory used to store some graph info and the local page rank value
    outlink = (int *)malloc( (sizeof(int) + sizeof(double))*nr_node
                            + sizeof(double)*n_size + sizeof(int)*n_size*(1+nr_node));
    
    
    
    /*get the array containing # of outlinks for each node in the graph*/
    MPI_Ibcast(outlink, nr_node, MPI_INT, MASTER, MPI_COMM_WORLD, &rreq[0]);
    
    pr = (double *)((int *)outlink + nr_node);
    //local_pr is the local page rank value the worker is computing
    local_pr = pr + nr_node;
    in_node_arr = (int*)((double*)local_pr + n_size);
    
    /*setup the array used to calculate page rank value.
     e.g. [2,1,3,3,1,2,3] means the first node the worker is responsible for has in_degree
     of 2, and node 1 and 3 pointing to it. the second node has in-degree of 3 and node 1,2,and 3 
     point to it.
     */
    MPI_Recv(&in_node_arr_size, 1, MPI_INT, MASTER, SIZE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    MPI_Irecv(in_node_arr, in_node_arr_size, MPI_INT, MASTER, IN_DEG_TAG, MPI_COMM_WORLD, &rreq[1]);
    
    /*wait all nessesary info has been set up so that we can start compute page rank value*/
    MPI_Waitall(2, rreq, MPI_STATUSES_IGNORE);
   
    /*------------------------------END OF SETTING UP THE COMPUTE ENVIRONMENT-----------------------*/
    
    while (1) {
        
        /*get the page rank values of last iteration from master*/
        MPI_Bcast(pr, nr_node, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
        
        /*calculate the local page rank values for nodes assigned to current worker*/
        worker_cal_PR(n_size, nr_node, dangl, in_node_arr,
                      in_node_arr_size, outlink, pr, local_pr);
        /*gather result to master*/
        MPI_Gatherv(local_pr, n_size, MPI_DOUBLE, NULL, NULL, NULL,
                    MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
        
        /*if not converge, continue, otherwise stop worker*/
        MPI_Bcast(&ctl_flag, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
        
        if (ctl_flag == CTL_STOP) {
            break;
        }
    }
    
    free(outlink);

}



/*page rank algorithm implementation*/
void worker_cal_PR(int n_size, int nr_node, double d,
                   const int in_node_arr[], int arr_size,
                   const int outlinks[], const double pr[], double local_pr[])
{
    int i, k, j = 0, nr_in; //nr_in the in-degree of current processing node
    int pos;
    double t = (d/(double)nr_node);
    d = 1-d;
    
    for (i = 0; i < n_size; i++) {
        //error: j go beyond the arr_size, just break.
        if (j >= arr_size) {
            break;
        }
        
        /* pr[i] = d/N + (1-d)*sum{(pr[j];j->i)/out(j)} */
        local_pr[i] = 0.0;
        nr_in = in_node_arr[j++];

        for (k = 0; k < nr_in; k++) {
            pos = in_node_arr[j++];
            local_pr[i] += pr[pos]/outlinks[pos];
        }
        
        local_pr[i] = t + d*local_pr[i];
    }

}



/*calculate the counts and displ array used by MPI_Gatherv*/
void calc_per_worker_jcounts_displ(int nr_prc, int nr_nodes, int counts[], int displ[])
{
    int w_rank;
    int beg, end, acc = 0;
    //master don't compute page rank value
    counts[0] = 0;
    displ[0] = 0;
    
    for (w_rank = 1; w_rank < nr_prc; w_rank++)
    {
        displ[w_rank] = acc;
        get_job_interval(&beg, &end, w_rank, nr_nodes, nr_prc);
        /*if w_rank worker is not assigned any task, assign count with 0*/
        if (beg == 0 && end == 0) {
            counts[w_rank] = 0;
            continue;
        }
        
        counts[w_rank] = end - beg + 1;
        acc += counts[w_rank];
    }
    

}

/* extact the info about which worker need which nodes in order to calculate the page rank values, and send them to coresponding worker. the message consist of an array of integers: e.g. [2,1,3,3,1,2,3] means the first node the worker is responsible for has  in_degree of 2, and node 1 and 3 pointing to it. the second node has in-degree of 3 and node 1,2,3 point to it.
 */
void setup_dependencies_info(inv_graph_t *g, int nr_prc, int nr_nodes)
{
    
    int beg, end, i;
    int w_rank;
    inv_adj_node_t *pn;
    ol_node_t oln;
    unsigned int c1;/*counter that always indicate the next element index in dep_info array to fill in data*/
    int *dep_info = malloc(sizeof(int)*( nr_nodes/nr_prc + 1 )*nr_nodes);/*ensure sufficient size*/
    MPI_Request req[2] = {MPI_REQUEST_NULL, MPI_REQUEST_NULL};
    
    for (w_rank = 1; w_rank < nr_prc; w_rank++)
    {
        MPI_Wait(&req[0], MPI_STATUS_IGNORE);
        c1 = 0;
        get_job_interval(&beg, &end, w_rank, nr_nodes, nr_prc);
        /*if w_rank worker is not assigned any task break, cause the remain workers are not any tasks*/
        if (beg == 0 && end == 0) {
            break;
        }
        beg--;
        end--;
        /*have to wait, because we can't modify dep_info array before the data is copied and sent by MPI*/
        MPI_Wait(&req[1], MPI_STATUS_IGNORE);
        /*now [begin, end] is the interval of nodes worker with rank w_rank is assigned to*/
        for (i = beg; i <= end; i++)
        {
            pn = g->array[i].head;
            oln = g->array[i].node;
            dep_info[c1++] = oln->nr_inlinks;
            while (pn != NULL) {
                dep_info[c1++] = pn->ol_node->pos;
                pn = pn->next;
            }
        }
        
        /*now the dep_info has data for w_rank worker*/
        MPI_Isend(&c1, 1, MPI_INT, w_rank, SIZE_TAG, MPI_COMM_WORLD, &req[0]);
        MPI_Isend(dep_info, c1, MPI_INT, w_rank, IN_DEG_TAG, MPI_COMM_WORLD, &req[1]);
    }
    MPI_Waitall(2, req, MPI_STATUSES_IGNORE);
    free(dep_info);
}


