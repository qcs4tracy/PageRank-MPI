//
//  pr_graph.h
//  PageRank-MPI
//
//  Created by QiuChusheng on 9/11/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//

#ifndef PageRank_MPI_pr_graph_h
#define PageRank_MPI_pr_graph_h
#include "hashtable.h"

typedef unsigned long g_index_t;
typedef struct ol_node* ol_node_t;
typedef struct inv_adj_node inv_adj_node_t;
typedef struct inv_adj_list inv_adj_list_t;
typedef struct inv_graph inv_graph_t;


/*define the node type with out link number recorded*/
struct ol_node {
    g_index_t index;/*index arbitrarily given by user*/
    unsigned int nr_outlinks;/*how many outlinks from this node*/
    unsigned int nr_inlinks;/*how many inlinks to this node*/
    int pos; /*the position in the array of linked graph implementation*/
};

/* element in the array of struct inv_graph*/
struct inv_adj_list {
    ol_node_t node; /*the node leading the adjacent list*/
    struct inv_adj_node *head;
};

/*the node within inv_adj_list */
struct inv_adj_node {
    ol_node_t ol_node; /* a pointer to the ol_node, containing needed info,in hashtable*/
    struct inv_adj_node *next;
};

/*graph struct:
 The inverse fashion of a graph.
 Every adjacent list in array is a list of nodes pointing to, instead of pointed by, the node.
 When calculating page rank value for a node, the only thing needed to be done is scanning other
 the ajacent list for the node.
 */
struct inv_graph {
    int size;
    inv_adj_list_t *array;
    mem_pool_t *pool;
};

/*functions related to hashtable oprations*/
hash_table_t *create_ol_node_table(int init_size, mem_pool_t *pool);
ol_node_t add_one_outlink(hash_table_t *ht, g_index_t index);
ol_node_t find_ol_node(hash_table_t *ht, g_index_t index);
unsigned int get_nr_outlinks(hash_table_t *ht, g_index_t index);
void free_ol_node_table(hash_table_t *ht);

/*functions related to graph operations*/
int init_graph(inv_graph_t *g, int init_size, mem_pool_t *pool);
void proc_edge(hash_table_t *ht, inv_graph_t *graph, g_index_t src, g_index_t dest);

/*setup graph from a file*/
int setup_graph_from_file(hash_table_t *ht, inv_graph_t *g, const char *path);

#endif









