#/bin/bash


function avg()
{
    echo "$1 to $2"
    sed -n "$1,$2 p" toplist_20220321_1 | awk -F'[' '{print $2}' | awk -F']' '{sum+=$1;} END { print "sum="sum; print "avg="sum/NR}'
    echo "=========================="
}

function test()
{
    inter=11
    #for loop in 126 141 156 171 186 201 216 231 246 336 366 396 
    #for loop in 6 36 66 96 126 156 186 216 246 276 306  
    for lp in {0..239} 
    do
        pt=$[1+13*$lp]
        loop=$[2+13*$lp]

        #echo "$pt and $loop"
        sed -n "$pt p" toplist_20220321_1
        avg $loop $[$loop+$inter]
    done
}

test
