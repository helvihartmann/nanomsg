#!/bin/bash

if [[ "$SLURMD_NODENAME" ]]; then

    readarray -t NODELIST < <(scontrol show hostname $SLURM_NODELIST)
    if [ "$SLURM_PROCID" == 0 ]; then
        echo ${NODELIST[*]}
        ./build/pubsubbm -u tcp://*:5555 -b 8589934592 -t server

    elif [ "$SLURM_PROCID" == 1 ]; then
        ./build/pubsubbm -u tcp://${NODELIST[0]}:5555 -n client0 -b 8589934592
    elif [ "$SLURM_PROCID" == 2 ]; then
        ./build/pubsubbm -u tcp://${NODELIST[0]}:5555 -n client1 -b 8589934592
    elif [ "$SLURM_PROCID" == 3 ]; then
        ./build/pubsubbm -u tcp://${NODELIST[0]}:5555 -n client2 -b 8589934592
    fi

else
    #started directly--------------------------------------------------------------------------------------------------------------
    srun -N 4 $0
fi

exit 0
