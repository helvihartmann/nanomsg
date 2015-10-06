#!/bin/bash

r=500
c=10
a=4
e=16777216

if [[ "$SLURMD_NODENAME" ]]; then

    readarray -t NODELIST < <(scontrol show hostname $SLURM_NODELIST)
    echo ${NODELIST[*]}
    if [ "$SLURM_PROCID" == 0 ]; then
        ./../build/pushpullbmts2 -t server -u tcp://* -r $r -c $c -a $a -e $e


    elif [ "$SLURM_PROCID" == 1 ]; then
#./build/pingpong send tcp://${NODELIST[0]}:5555 "Hello die Welt geht unter"
        ./../build/pushpullbmts2 -t client -u tcp://${NODELIST[0]}ib0 "" -r $r -c $c -a $a -e $e
#        ./../build/pushpullbm -t client -u tcp://${NODELIST[0]}ib2:5555 "" -r 100000

    fi

else
    #started directly--------------------------------------------------------------------------------------------------------------
    srun -N 2 $0
fi

exit 0
