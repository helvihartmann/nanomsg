#!/bin/bash

end=4;

if [[ "$SLURMD_NODENAME" ]]; then

    readarray -t NODELIST < <(scontrol show hostname $SLURM_NODELIST)
    if [ "$SLURM_PROCID" == 0 ]; then
        echo ${NODELIST[*]}
        ./../build/surveybm -u tcp://*:5555 -b 2048 -t server
    fi

    for ((n=1;n<$end;n++)); do
        if [ "$SLURM_PROCID" == $n ]; then
            name="client"$n

            echo "process " $n $name
            ./../build/surveybm -u tcp://${NODELIST[0]}:5555 -n $name -b 2048
        fi
    done

else
    #started directly--------------------------------------------------------------------------------------------------------------
    srun -N $end $0
fi

exit 0
