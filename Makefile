#this is a makefile
CC=$(shell which gcc)
MPICC=$(shell which mpicc)
CFLAGS = -c -Wall -std=gnu99
LDFLAGS=
OBJS=palloc.o \
	 hashtable.o \
	 pr_graph.o \
	 page_rank.o \
	 mpi_pr.o \
	 args_parse.o \
	 page_rank_mpi.o 

SEQ_OBJS=palloc.o \
		 hashtable.o \
		 pr_graph.o \
		 page_rank.o \
		 args_parse.o \
		 page_rank_seq.o

all: page_rank_mpi page_rank_seq

page_rank_mpi: $(OBJS)
	$(MPICC) $(LDFLAGS) $^ -o $@

page_rank_seq: $(SEQ_OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

mpi_pr.o: mpi_pr.c
	$(MPICC) $(CFLAGS) $^ -o $@

#implicit rule
%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(OBJS) $(SEQ_OBJS)
	rm -f ./page_rank_mpi ./page_rank_seq

.PHONY:clean

