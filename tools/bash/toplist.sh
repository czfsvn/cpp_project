#/bin/bash

path="../../build"
#VER="debug"
VER="release" 

function testtoplist()
{
    #cmd="../build/release/bin/example -c $1 -t $2 -o $3"
    cmd="$path/$VER/bin/example -o $1 -t $2 -s $3"
    echo "container: $2, times: $3 function: $1"
    echo $cmd

    for loop in 1 2 3 4 5 6 7 8 9 10 11 12
    do
        $cmd
        sleep 3
    done
}

function call_test()
{
    testtoplist $1 skip $2
    echo "====================================="
    testtoplist $1 set $2
    echo "====================================="
    testtoplist $1 vec $2
}

function call_looptimes()
{
    for loop in 100 500 1000 2000 5000 8000 10000 20000 30000 50000 60000 70000 80000 90000 100000 
    do
        echo "loop times: $loop -------------------------------------"
        call_test $1 $loop 
        sleep 10
    done
}

function call_looptimes1()
{
    for loop in 100 500 1000 2000 5000 8000 10000 20000 30000 50000
    do
        echo "loop times: $loop -------------------------------------"
        call_test $1 $loop 
        sleep 10
    done
}


function call_loopskip()
{
    for loop in 50000 80000 100000 120000 140000 160000 180000 200000
    do
        echo "loop times: $loop -------------------------------------"  
        testtoplist $1 skip $loop
        sleep 10
    done
}

function call_allfunc()
{

    for loop in test_update test_getDataByRank test_getRankByKey test_getRevRankByKey  
    do
        echo "loop times: $loop -------------------------------------"  
        echo "call_looptimes1 $loop > $loop"
        call_looptimes1 $loop 
        sleep 10
    done

    for loop in test_deleteByKey test_deleteByRank test_deleteByRangedRank test_forEachByRangedRank
    do
        echo "loop times: $loop -------------------------------------"  
        echo "call_looptimes1 $loop > $loop"
        call_looptimes1 $loop
        sleep 10
    done


}
#call_looptimes1 $1
call_allfunc
