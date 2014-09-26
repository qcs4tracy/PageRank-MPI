#!/bin/bash
#the shell file is used to generate data used to draw a graph in matlab
MPI_PROG=./page_rank_mpi
SEQ_PROG=./page_rank_seq
MAKEFILE=Makefile
MPIRUN=$(which mpirun)
OUT=./data.txt
T_MPI=
T_SEQ=

if [[ ! -e ${MPI_PROG} || ! -e ${SEQ_PROG} ]]; then

    if [ ! -e ${MAKEFILE} ]; then
        echo "there's no Makefile under the folder!"
        exit -1
    fi

    make

    if [ -z ${MPIRUN} ]; then
        echo "**error:mpirun is not installed"
        exit -1
    fi

fi

if [[ -x ${MPI_PROG} && -x ${SEQ_PROG} ]]; then

    echo '' > ${OUT}

    for((i=2;i<=8;i++))
    do
        T_MPI=$(${MPIRUN} -n $i ${MPI_PROG} 2>&1 1>/dev/null)
        T_SEQ=$(${SEQ_PROG} 2>&1 1>/dev/null)
        echo "${T_MPI} ${T_SEQ}" >> ${OUT}
    done
    echo "finish.."
    echo "the data is in data.txt"
else

   echo "can't execute excutables."

fi