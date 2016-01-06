#!/bin/bash

if [[ "$SLURMD_NODENAME" ]]; then

    ./../build/agent > agent-$SLURM_NODELIST.out


else
    #started directly--------------------------------------------------------------------------------------------------------------
#srun -N 2 $0
    srun -n 1 --nodelist=node0 $0
fi

exit 0
