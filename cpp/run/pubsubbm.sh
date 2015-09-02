#!/bin/bash

end=4;
r=1000;

if [[ "$SLURMD_NODENAME" ]]; then

    readarray -t NODELIST < <(scontrol show hostname $SLURM_NODELIST)
    if [ "$SLURM_PROCID" == 0 ]; then
        echo ${NODELIST[*]}
        ./../build/pubsubbm -u tcp://* -t server -s $((end-1)) -r $r
    fi

    for ((n=1;n<$end;n++)); do
        if [ "$SLURM_PROCID" == $n ]; then
            #name="client"$n

            echo "process " $n $name
            ./../build/pubsubbm -u tcp://${NODELIST[0]} -n $((n-1)) -s $((end-1)) -r $r
        fi
    done

else
    #started directly--------------------------------------------------------------------------------------------------------------
    srun -N $end $0
fi

exit 0
