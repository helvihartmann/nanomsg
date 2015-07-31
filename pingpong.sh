#!/bin/bash

if [[ "$SLURMD_NODENAME" ]]; then

    if [ "$SLURM_PROCID" == 0 ]; then
        ./build/pingpong receive tcp://*:5555


    elif [ "$SLURM_PROCID" == 1 ]; then
        ./build/pingpong send tcp://node0:5555 Hello
        ./build/pingpong send tcp://node0:5555 ""

    fi

else
    #started directly--------------------------------------------------------------------------------------------------------------
    srun -w node0,node1 -n 2 $0
fi

exit 0
