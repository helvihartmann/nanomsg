#!/bin/bash

end=4;

if [[ "$SLURMD_NODENAME" ]]; then

    readarray -t NODELIST < <(scontrol show hostname $SLURM_NODELIST)
    if [ "$SLURM_PROCID" == 0 ]; then
        echo ${NODELIST[*]}
        ./../build/pubsubbm -u tcp://*:5555 -b 8589934592 -t server -s $((end-1))
    fi

    for ((n=1;n<$end;n++)); do
        if [ "$SLURM_PROCID" == $n ]; then
            name="client"$n

            echo "process " $n $name
            ./../build/pubsubbm -u tcp://${NODELIST[0]}:5555 -n $name -b 8589934592 -s $((end-1))
        fi
    done

else
    #started directly--------------------------------------------------------------------------------------------------------------
    srun -N $end $0
fi

exit 0
