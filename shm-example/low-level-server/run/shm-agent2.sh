#!/bin/bash

if [[ "$SLURMD_NODENAME" ]]; then

    readarray -t NODELIST < <(scontrol show hostname $SLURM_NODELIST)
    echo ${NODELIST[*]}
    if [ "$SLURM_PROCID" == 0 ]; then
        ./../build/agent


    elif [ "$SLURM_PROCID" == 1 ]; then
        ./../build/shm-writer

    elif [ "$SLURM_PROCID" == 2 ]; then
        ./../build/shm-reader
    fi

else
    #started directly--------------------------------------------------------------------------------------------------------------
#srun -N 2 $0
    srun -n 3 --nodelist=node0 $0
fi

exit 0
