#!/bin/bash

end=4;
clients=$((end-1))
if [[ "$SLURMD_NODENAME" ]]; then

    readarray -t NODELIST < <(scontrol show hostname $SLURM_NODELIST)
    if [ "$SLURM_PROCID" == 0 ]; then
        echo ${NODELIST[*]}
        ./../build/surveybm -u tcp://*:5555 -t server
    fi

    for ((n=1;n<$clients;n++)); do
        if [ "$SLURM_PROCID" == $n ]; then
            name="client"$n

            echo "process " $n $name
            ./../build/surveybm -u tcp://${NODELIST[0]}:5555 -n $name
        fi
    done

    if [ "$SLURM_PROCID" == $((end-1)) ]; then
        echo "idler"
        ./../build/surveybm -u tcp://${NODELIST[0]}:5555 -t idler
    fi

else
    #started directly--------------------------------------------------------------------------------------------------------------
    srun -N $end $0
fi

exit 0
