//
//  args_parse.c
//  PageRank-MPI
//
//  Created by QiuChusheng on 9/14/14.
//  Copyright (c) 2014 QiuChusheng. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include "mpi_pr.h"
#include "args_parse.h"

#define O_THRESHOLD 1
#define O_FILEPATH 2
#define O_DANGLING 3



static int check_options(const char *option)
{
    static char* valid_options[3] = {"-t", "-f", "-d"};
    
    for (int i = 0; i < sizeof(valid_options); i++) {
        if(strcmp(option, valid_options[i]) == 0) return i+1;
    }
    
    return -1;
}

void parse_args(int argc, char *argv[], double *threshold, double *d, char **pfn)
{
    
    int i = 1, w;
    for (; i < argc; i++) {
        if((w = check_options(argv[i])) > 0)
        {
            if (i == (argc-1)) {
                break;
            }
            
            switch (w) {
                    
                case O_THRESHOLD:
                    *threshold = atof(argv[++i]);
                    if ((*threshold) > 1.0 || (*threshold) <= 0.0) {
                        *threshold = DEF_THRED;//default is 0.01
                    }
                    break;
                    
                case O_DANGLING:
                    *d = atof(argv[++i]);
                    if ((*d) > 1.0 || (*d) <= 0.0) {
                        *d = dangling;//default is 0.15
                    }
                    break;
                    
                case O_FILEPATH:
                    *pfn = argv[++i];
                    break;
                    
                default:
                    break;
            }
        }
    }
    
}

