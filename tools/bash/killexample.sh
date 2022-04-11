#/bin/bash

condition=1
while true
do
    pid=`ps aux | grep example | grep -v vi | grep -v grep | awk '{print $2}'`
    if [ -n "$pid" ]
    then
        echo "kill -9 $pid"
        kill -9 $pid
    fi
    sleep 1
done
