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
    for loop in 126 141 156 171 186 201 216 231 246
    do
        avg $loop $[$loop+$inter]
    done
}

test
