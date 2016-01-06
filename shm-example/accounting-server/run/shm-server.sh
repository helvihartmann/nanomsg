#!/bin/bash
normalagents=2
n=$((1+normalagents+1))

if [[ "$SLURMD_NODENAME" ]]; then

#echo $SLURMD_NODENAME
#./../build/server

    readarray -t NODELIST < <(scontrol show hostname $SLURM_NODELIST)
    if [ "$SLURM_PROCID" == 0 ]; then
        echo ${NODELIST[*]}
        ./../build/server -n $((n-1))
    fi
    for id in $(seq 1 $normalagents)
    do
        if [ "$SLURM_PROCID" == $id ]; then
            ./../build/agent -i $id > agent-$SLURMD_NODENAME.out
        fi

    done
    if [ "$SLURM_PROCID" == $((normalagents+1)) ]; then
        ./../build/agent -i $((normalagents+1)) -t 4 > agent-$SLURMD_NODENAME.out
    fi

else
    #started directly---------------------------------------------------------------

    srun -N $n $0
fi

exit 0
