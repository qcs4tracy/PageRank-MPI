//
//  hashtable.h
//  PageRank-MPI
//
//  Created by QiuChusheng on 9/11/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//

#ifndef PageRank_MPI_hashtable_h
#define PageRank_MPI_hashtable_h

#include "palloc.h"

/*generic types for hashtable*/
typedef void *ht_key;
typedef void  *ht_elem;
typedef struct hash_table hash_table_t;
typedef struct bucket bucket_t;

/*hashtable struct*/
struct hash_table {
    int size;			/* size of array */
    int num_elems;		/* # of elements in hashtable */
    bucket_t **buckets;			/* len(buckets) == size */
    ht_key (*elem_key)(ht_elem e); /* function pointer used to extract keys from ht_elem */
    int (*equal)(ht_key key1, ht_key key2); /* comparing keys */
    int (*hash)(ht_key key, int limit);	       /* hashing keys */
    mem_pool_t *pool; /*memory pool assigned to hashtable*/
};

/*the type of function used to free elements*/
typedef void (*elem_free_func)(ht_elem e);

hash_table_t *create_table(int init_size,
                           ht_key (*elem_key)(ht_elem e),
                           int (*equal)(ht_key k1, ht_key k2),
                           int (*hash)(ht_key k, int l), mem_pool_t *pool);

ht_elem hash_table_lookup(hash_table_t *ht, ht_key key);

/*how many elements are there in hashtable*/
int nr_elem(hash_table_t *ht);

ht_elem hash_table_insert_update(hash_table_t *ht, ht_elem elem);

void free_hash_table(hash_table_t *ht, elem_free_func elem_free);

#endif
