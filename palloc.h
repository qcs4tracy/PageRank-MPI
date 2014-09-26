//
//  palloc.h
//
//  Created by QiuChusheng on 9/11/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//

#ifndef PageRank_MPI_palloc_h
#define PageRank_MPI_palloc_h
#include <sys/types.h>
#include <stddef.h>

#define MEM_OK 0x0000001
#define MEM_FAILED 0x00000002
#define MEM_POOL_DEFAUT_SIZE (16 * 1024)

/*basic data structures*/
typedef struct mem_pool_s mem_pool_t;
typedef struct mem_pool_data_s mem_pool_data_t;
typedef struct mem_pool_large_s mem_pool_large_t;

/*this struct is included in the mem_pool_t struct, used to store info about a block in the pool chain*/
struct mem_pool_data_s {
    mem_pool_t *next; /*the next block of memory pool*/
    u_char *last; /*the last position of memory in this block*/
    u_char *end; /*the limit position of memory in this block*/
    u_int failed; /*how many times allocation request have failed, cause by lack of memory left in this block*/
    
};

/*at the begining of every block*/
struct mem_pool_s {
    mem_pool_data_t d;/*see above*/
    size_t max; /*max of memory that can be allocated once*/
    mem_pool_large_t *large; /*used to process large continuous memory allocation request*/
    mem_pool_t *current;/*the current pool block to be used for allocation*/
    /*
    log_t *log;
     */
};

/*large memory area linked list*/
struct mem_pool_large_s {
    void *alloc;
    mem_pool_large_t *next;
};

/*normal memory allocations*/
void *mem_alloc(size_t size/*, log_t *log */);
void *mem_calloc(u_int nr, size_t size/*, log_t *log */);

/*memory allocation using memory pool*/
mem_pool_t *mem_create_pool(size_t size/*, log_t *log */);
void mem_destroy_pool(mem_pool_t *pool);
void mem_reset_pool(mem_pool_t *pool);

void *mem_palloc(mem_pool_t *pool, size_t size);
void *mem_pnalloc(mem_pool_t *pool, size_t size);
void *mem_pcalloc(mem_pool_t *pool, u_int nr, size_t size);

/* void *mem_pmemalign(mem_pool_t *pool, size_t size, size_t alignment); */
u_int mem_pfree(mem_pool_t *pool, void *p);

extern mem_pool_t *global_pool;

#endif
