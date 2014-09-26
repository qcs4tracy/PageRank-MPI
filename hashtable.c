//
//  hashtable.c
//  PageRank-MPI
//
//  Created by QiuChusheng on 9/11/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.

// a generic hashtable implementation

#include <stdio.h>
#include <stdlib.h>
#include "hashtable.h"

#define TRUE 1
#define FALSE 0


/*bucket inside hashtable*/
struct bucket {
    ht_elem elem;
    struct bucket *next;
};


/*look up a bucket for key*/
static bucket_t *look_up_bucket(hash_table_t *ht, ht_key key);


/*create a hash table with init_size as initial size*/
hash_table_t *create_table(int init_size,
                           ht_key (*elem_key)(ht_elem e),
                           int (*equal)(ht_key k1, ht_key k2),
                           int (*hash)(ht_key k, int l), mem_pool_t *pool)
{
    /* a memory pool may applied to avoid frequently call malloc/calloc syscalls*/
    bucket_t ** bcks;
    hash_table_t *ht;
    if (pool != NULL) {
        bcks = mem_pcalloc(pool, init_size, sizeof(bucket_t *));
        ht = mem_palloc(pool, sizeof(hash_table_t));
    } else {
        bcks = calloc(init_size, sizeof(bucket_t *));
        ht = malloc(sizeof(hash_table_t));
    }
    
    if (bcks == NULL || ht == NULL) {
        return NULL;
    }
    
    ht->size = init_size;
    ht->num_elems = 0;
    ht->buckets = bcks;	//use calloc with all bucket pointer initialized to NULL
    ht->elem_key = elem_key;
    ht->equal = equal;
    ht->hash = hash;
    ht->pool = pool;
    return ht;
}


/* look up an element key *key* in the table with */
ht_elem hash_table_lookup(hash_table_t *ht, ht_key key)
{
    bucket_t *pBck = look_up_bucket(ht, key);
    return pBck != NULL? pBck->elem: NULL;
}

/*look up a bucket for key*/
static bucket_t *look_up_bucket(hash_table_t *ht, ht_key key)
{
    int index;
    bucket_t *pBck;
    index = ht->hash(key, ht->size);
    pBck = ht->buckets[index];
    
    while (pBck != NULL) {
        
        if ( ht->equal(ht->elem_key(pBck->elem), key) ) {
            return pBck;
        }
        
        pBck = pBck->next;
    }
    
    return NULL;
}

/*insert an element into hashtable, or update it if it already exist*/
ht_elem hash_table_insert_update(hash_table_t *ht, ht_elem elem)
{
    int index;
    ht_key k;
    bucket_t *pBck, *pNewBck;
    ht_elem old;
    k = ht->elem_key(elem);
    pBck = look_up_bucket(ht, k);
    
    if (pBck == NULL) {/*insert an new element*/
        /*there's a pool or not*/
        if (ht->pool == NULL) {
            pNewBck = malloc(sizeof(bucket_t));
        } else { pNewBck = mem_palloc(ht->pool, sizeof(bucket_t)); }
        
        pNewBck->elem = elem;
        index = ht->hash(k, ht->size);
        pNewBck->next = ht->buckets[index];
        ht->buckets[index] = pNewBck;
        ht->num_elems++;
        return NULL;
    }
    /* update the existing element */
    old = pBck->elem;
    pBck->elem = elem;
    
    return old;
}

/*function used to free one bucket list*/
void free_bucket_list(bucket_t *pBck, elem_free_func elem_free)
{
    bucket_t *tmp;
    /**/
    while (pBck != NULL) {
        if (pBck->elem != NULL && elem_free != NULL) { elem_free(pBck->elem); }
        tmp = pBck->next;
        free(pBck);
        pBck = tmp;
    }
}

/*function used to free hashtable*/
void free_hash_table(hash_table_t *ht, elem_free_func elem_free)
{
    int size = ht->size;
    /*free all bucket list*/
    for (int i=0; i < size; i++) {
        free_bucket_list(ht->buckets[i], elem_free);
    }
    /*free the buckets field*/
    free(ht->buckets);
    /*free the table struct*/
    free(ht);
}


int nr_elem(hash_table_t *ht)
{
    return ht->num_elems;
}













