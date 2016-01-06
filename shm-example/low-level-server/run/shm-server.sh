#!/bin/bash
n=3

if [[ "$SLURMD_NODENAME" ]]; then

#echo $SLURMD_NODENAME
#./../build/server

    readarray -t NODELIST < <(scontrol show hostname $SLURM_NODELIST)
    if [ "$SLURM_PROCID" == 0 ]; then
        echo ${NODELIST[*]}
        ./../build/server -n $((n-1))
    fi
    for id in $(seq 1 $n)
    do
        if [ "$SLURM_PROCID" == $id ]; then
            ./../build/agent -i $id > agent-$SLURMD_NODENAME.out
        fi

    done

else
    #started directly---------------------------------------------------------------

    srun -N 3 $0
fi

exit 0
