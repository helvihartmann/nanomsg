#!/bin/bash

if [[ "$SLURMD_NODENAME" ]]; then

    if [ "$SLURM_PROCID" == 0 ]; then
        ./build/pingpong receive ipc:///tmp/pipeline.ipc


    elif [ "$SLURM_PROCID" == 1 ]; then
        ./build/pingpong send ipc:///tmp/pipeline.ipc Hello
        ./build/pingpong send ipc:///tmp/pipeline.ipc ""

    fi

else
    #started directly--------------------------------------------------------------------------------------------------------------
    srun -w node0,node0 -n 2 $0
fi

exit 0
