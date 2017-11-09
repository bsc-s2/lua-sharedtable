#! /bin/bash

function stop_openresty() {
    path=~/openresty/nginx/logs/nginx.pid

    pid=$(cat ${path})
    echo "pid: " ${pid}

    if [[ $1 == "stop" ]]; then
        signal="QUIT"
    else
        signal="HUP"
    fi

    kill -s ${signal} ${pid}
}

if [[ $1 == "start" ]]; then
    echo "starting openresty"
    sbin/openresty -c nginx.conf -p ./

elif [[ $1 == "stop" ]]; then
    echo "stopping openresty"
    stop_openresty stop

elif [[ $1 == "r" ]]; then
    echo 'restart'
    stop_openresty restart
fi
