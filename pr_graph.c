//
//  pr_graph.c
//  PageRank-MPI
//
//  Created by QiuChusheng on 9/11/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "pr_graph.h"
#include "palloc.h"

/* strip all white spaces within a string */
static void strip_whitespaces(char *buff);

/*global variable used to alloc position for different node in the array of the linked graph implementation*/
int next_pos = 0;

#define NEW_OL_NODE() ((ol_node_t)malloc(sizeof(struct ol_node)))

/*function to extract key from element*/
ht_key ol_elem_key(ht_elem e)
{
    return (ht_key)((ol_node_t)e)->index;
}

/*function to compare keys*/
int ol_equal_key(ht_key key1, ht_key key2)
{
    return ((g_index_t)key1) == ((g_index_t) key2);
}

/* integer hash function from Thomas Wang's page */
int int32hash(ht_key s, int m)
{
    unsigned int a = (unsigned int)s;
    a = (a ^ 61) ^ (a >> 16);
    a = a + (a << 3);
    a = a ^ (a >> 4);
    a = a * 0x27d4eb2d;
    a = a ^ (a >> 15);
    a = a % m;
    int h = (int)a;
    if (h < 0) h += m;
    return a;
}

/* free a elem*/
void ol_elem_free(ht_elem e)
{
    free(e);
}

/*wrapper of create_hash_table*/
hash_table_t *create_ol_node_table(int init_size, mem_pool_t *pool)
{
    return create_table(init_size, ol_elem_key, ol_equal_key, int32hash, pool);
}

/* add #nr outlinks to node with index *index* */
ol_node_t add_outlink_nr(hash_table_t *ht, g_index_t index, unsigned int nr)
{
    ol_node_t n = (ol_node_t)hash_table_lookup(ht, (ht_key)index);
    if (n != NULL) {
        n->nr_outlinks += nr;
        return n;
    }
    
    if (ht->pool != NULL) {
        n = (ol_node_t)mem_palloc(ht->pool, sizeof(struct ol_node));
    } else { n = NEW_OL_NODE(); }
    
    n->index = index;
    n->pos = next_pos++;
    n->nr_outlinks = nr;
    n->nr_inlinks = 0;
    hash_table_insert_update(ht, n);
    return n;
}

/*add one outlink to index*/
ol_node_t add_one_outlink(hash_table_t *ht, g_index_t index)
{
    return add_outlink_nr(ht, index, 1);
}

/*get # of outlinks of node index*/
unsigned int get_nr_outlinks(hash_table_t *ht, g_index_t index)
{
    ol_node_t n = (ol_node_t) hash_table_lookup(ht, (ht_key)index);
    if (n == NULL) return 0;
    return n->nr_outlinks;
}

/*find the ol_node with index, an pointer to an element in hashtable will be returned*/
ol_node_t find_ol_node(hash_table_t *ht, g_index_t index)
{
    return (ol_node_t)hash_table_lookup(ht, (ht_key)index);
}

/*free the hash table*/
void free_ol_node_table(hash_table_t *ht)
{
    free_hash_table(ht, ol_elem_free);
}


/*add to adj list*/
int add_adj_node(inv_adj_list_t *list, ol_node_t node, mem_pool_t *pool)
{
    inv_adj_node_t *inv_node;
    
    if (pool != NULL) {
        inv_node = mem_palloc(pool, sizeof(inv_adj_node_t));
    } else {  inv_node = malloc(sizeof(inv_adj_node_t)); }
   
    /*allocate memory failed*/
    if (inv_node == NULL) {
        return -1;
    }
    
    inv_node->ol_node = node;
    
    /*insert the node in the front of the node list*/
    inv_node->next = list->head;
    list->head = inv_node;
    return 0;
}

/* edge processing */
void proc_edge(hash_table_t *ht, inv_graph_t *graph, g_index_t src, g_index_t dest)
{
    ol_node_t sn, dn;
    
    /*add one outlink to src node*/
    sn = add_one_outlink(ht, src);
    
    /*check if the dest exist*/
    dn = find_ol_node(ht, dest);
    
    /*if dest node has not been added to hashtable*/
    if (dn == NULL) {
        dn = add_outlink_nr(ht, dest, 0);/*add to hashtable with outlink set to 0*/
        graph->array[dn->pos].node = dn;
    }
    
    /*increment the dest node's nr_inlink*/
    dn->nr_inlinks++;
    
    if (graph->array[sn->pos].node == NULL) {
        graph->array[sn->pos].node = sn;
    }
    
    add_adj_node(&(graph->array[dn->pos]), sn, graph->pool);/*add the node to adjacent list*/
    return;
    
}


/*initialize the graph*/
int init_graph(inv_graph_t *g, int init_size, mem_pool_t *pool)
{
    inv_adj_list_t *arr;
    
    if (pool == NULL) {
        arr = calloc(init_size, sizeof(inv_adj_list_t));
    } else { arr = mem_pcalloc(pool, init_size, sizeof(inv_adj_list_t)); }
    
    if (arr == NULL) {
        return -1;
    }
    
    g->array = arr;
    g->size = init_size;
    g->pool = pool;
    
    return 0;
}


/*function used to strip the white spaces inside a string*/
static void strip_whitespaces(char *buff)
{
    int i=0, j=0;
    int len = (int)strlen(buff);
    
    while (i != len) {
        if (buff[i] != ' ')
            buff[j++] = buff[i];
        i++;
    }
    
    buff[j] = '\0';
}

/*set up the graph from file*/
int setup_graph_from_file(hash_table_t *ht, inv_graph_t *g, const char *path)
{
    FILE *fp =  fopen(path, "r");
    char *line = NULL, *tmp = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    g_index_t src, dest;
    
    if (fp == NULL) {
        fprintf(stderr, "open file %s occur an error!\n", path);
        perror("[ERROR]");
        return -1;
    }
    
    while ((linelen = getline(&line, &linecap, fp)) > 0)
    {
        strip_whitespaces(line);
        if (*line == '(') {
            line++;
            tmp = line;
            
            /*split (a,b) into two string seperated by comma*/
            while (*tmp != ',' || *tmp == '\0') tmp++;
            if (*tmp == '\0') {//skip the illegal format line
                continue;
            }
            *tmp = '\0';
            /*source node label*/
            src = atoi(line);
            
            line = ++tmp;
            /*find the ending*/
            while (*tmp != ')' || *tmp == '\0') tmp++;
            if (*tmp == '\0') {//skip the illegal format line
                continue;
            }
            *tmp = '\0';
            /*destination node label*/
            dest = atoi(line);
            
            proc_edge(ht, g, src, dest);

        }
        line = NULL;
        
    }
    
    fclose(fp);
    return 0;
}


