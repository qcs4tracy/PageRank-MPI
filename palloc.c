//
//  palloc.c
//
//  Created by QiuChusheng on 9/11/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//
/*
 * a memory pool implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "palloc.h"

#define mem_memzero(p, size) memset(p, 0, size)

#define MAX_ALLOC_FROM_POOL ((4 * 1024) - 1)
#define MEM_POOL_FAIL_THREHOLD 4

/*these functions are used only in this file, unaccessible from outside the file*/
static void *mem_palloc_block(mem_pool_t *pool, size_t size);
static void *mem_palloc_large(mem_pool_t *pool, size_t size);

mem_pool_t *global_pool = NULL;

/*simple wrapper of malloc function*/
void *mem_alloc(size_t size/*, log_t *log */){
    
    void *p;
    p = malloc(size);
    
    if (p == NULL){/*log goes here*/}
    
    return p;
}


/*simpler wrapper */
void *mem_calloc(u_int nr, size_t size/*, log_t *log */){

    void *p;
    p = mem_alloc(size*nr);
    
    if (p != NULL) memset(p, 0, size);
    
    return p;
}

/*create a memory pool with initial size of *size* */
mem_pool_t *mem_create_pool(size_t size/*, log_t *log */){

    mem_pool_t *p;
    p = mem_alloc(size);
    
    if(p == NULL){
        /*log goes here*/
        return NULL;
    }
    /*initialize mem_pool_t struct fields*/
    p->d.last = (u_char *)p + sizeof(mem_pool_t);
    p->d.end = (u_char *)p + size;
    p->d.next = NULL;
    p->d.failed = 0;
    
    /*the actual size of the pool, cause the first few bytes are used by the mem_pool_t struct*/
    size = size - sizeof(mem_pool_t);
    /*the maximum of memory that can be allocated from memory pool at a time
    very large memory allocations are processed seperately.
     */
    p->max = size < MAX_ALLOC_FROM_POOL? size: MAX_ALLOC_FROM_POOL;
    
    /*the block currently used (pool is implemented as a chain of blocks of type mem_pool_t)*/
    p->current = p;
    p->large = NULL;
    
    return p;
}

/*destroy a memory pool and release the memory*/
void mem_destroy_pool(mem_pool_t *pool){

    mem_pool_t *p, *n;
    mem_pool_large_t *l;
    
    /*release the large memory areas*/
    for (l = pool->large; l; l = l->next) {
        if (l->alloc != NULL) {
            free(l->alloc);
        }
    }
    
    /*release the memory pool blocks*/
    for (p = pool, n = p->d.next; /*void*/; p = n, n = p->d.next) {
        
        free(p);
        
        if (n == NULL) {
            break;
        }
    }

}


/*reset the memory pool so that it can be resued without re-create a pool again*/
void mem_reset_pool(mem_pool_t *pool){

    mem_pool_t *p;
    mem_pool_large_t *l;
    
    /* free the large mem*/
    for (l = pool->large; l; l = l->next) {

        if (l->alloc != NULL) {
            free(l->alloc);
        }
    }
    
    pool->large = NULL;
    
    /* reset the pool memory*/
    for (p = pool; p; p = p->d.next) {
        p->d.last = (u_char *)p + sizeof(mem_pool_t);
        p->d.failed = 0;
    }
    /*reset current pointer*/
    pool->current = pool;

}

/* allocate *size* memory from *pool* */
void *mem_palloc(mem_pool_t *pool, size_t size){

    u_char *m;
    mem_pool_t *p;
    
    /*if the size less than the limit*/
    if (size <= pool->max) {
        
        p = pool->current;
        /*search through the block chain to find available memory*/
        do {
            m = p->d.last;
            if ((size_t)(p->d.end - m) >= size) {
                p->d.last += size;
                return m;
            }
            
            p = p->d.next;
            
        } while (p);
        /*add a new pool block to the pool and allocate memory from that block*/
        return mem_palloc_block(pool, size);
    }
    
    /*exceed the limitation, deal the request with large memory allocation*/
    return mem_palloc_large(pool, size);

}

/*allocation a block from system and add it to the memory pool*/
static void *mem_palloc_block(mem_pool_t *pool, size_t size){
    
    u_char *m;
    size_t psize;
    mem_pool_t *p, *new, *current;
    
    /*calculate the per block size.*/
    psize = (size_t)((unsigned long)pool->d.end - (unsigned long)pool);
     
    /*if this failed*/
    m = mem_alloc(psize);
    
    if (m == NULL) return NULL;
    
    /*initialize the new pool block*/
    new = (mem_pool_t *) m;
    new->d.end = m + psize;
    m += sizeof(mem_pool_t);
    new->d.last = m + size;
    new->d.failed = 0;
    new->d.next = NULL;
    /*the only point to change the first pool block to be used for allocations
      ie. the current pointer of the mem_pool_t struct.
     */
    current = pool->current;
    for (p = current; p->d.next; p = p->d.next) {
        /*if this block failed too many times attempting to satisfy requests,
          change the current to its next block in the pool.
         */
        if (p->d.failed++ > MEM_POOL_FAIL_THREHOLD) {
            current = p->d.next;
        }
    }
    /*add the new block to the list*/
    p->d.next = new;
    /*set the current pointer of the pool*/
    pool->current = current? current: new;
    
    return m;
}

/*allocations of large memory areas*/
static void *mem_palloc_large(mem_pool_t *pool, size_t size){
    
    void *p;
    u_int16_t n;
    mem_pool_large_t *large;
    
    p = mem_alloc(size);
    if(p == NULL) return NULL;
    
    n = 0;
    
    for (large = pool->large; large; large = large->next) {
        /*reusable mem_pool_large_t struct*/
        if (large->alloc == NULL) {
            large->alloc = p;
            return p;
        }
        /*3 times, stop and allocate a new mem_pool_large_t struct for p*/
        if (n++ > 3) {
            break;
        }
    }
    /*allocate a mem_pool_large_t struct from pool*/
    large = mem_palloc(pool, sizeof(mem_pool_large_t));
    if (large == NULL) {
        free(p);
        return NULL;
    }
    /*add to large linked list of the pool*/
    large->alloc = p;
    large->next = pool->large;
    pool->large = large;
    
    return p;
    
}

/*free large memory areas*/
u_int mem_pfree(mem_pool_t *pool, void *p){

    mem_pool_large_t *l;
    for (l = pool->large; l; l = l->next) {
        if (l->alloc == p) {
            free(l->alloc);
            l->alloc = NULL;
            return MEM_OK;
        }
    }
    
    return MEM_FAILED;
    
}

/*the allocate memory of size *size* from */
void *mem_pcalloc(mem_pool_t *pool, u_int nr, size_t size){

    void *p;
    p = mem_palloc(pool, size*nr);
    if (p) {
        mem_memzero(p, size);
    }
    
    return p;
}

/* palloc without memory alignment */
void *mem_pnalloc(mem_pool_t *pool, size_t size){
    /* now palloc is not align memory */
    return mem_palloc(pool, size);

}




