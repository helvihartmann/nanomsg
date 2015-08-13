#!/bin/bash

if [[ "$SLURMD_NODENAME" ]]; then

    readarray -t NODELIST < <(scontrol show hostname $SLURM_NODELIST)
    if [ "$SLURM_PROCID" == 0 ]; then
        echo ${NODELIST[*]}
        ./build/pubsubbm server tcp://*:5555

    elif [ "$SLURM_PROCID" == 1 ]; then
        ./build/pubsubbm client tcp://${NODELIST[0]}:5555 client0
    elif [ "$SLURM_PROCID" == 2 ]; then
        ./build/pubsubbm client tcp://${NODELIST[0]}:5555 client1
    elif [ "$SLURM_PROCID" == 3 ]; then
        ./build/pubsubbm client tcp://${NODELIST[0]}:5555 client2
    fi

else
    #started directly--------------------------------------------------------------------------------------------------------------
    srun -N 4 $0
fi

exit 0
