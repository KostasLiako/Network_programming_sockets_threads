#!/bin/bash

gcc -o travelMonitorClient travelMonitorClient.c citizen.c list.c travelFunctions.c skipList.c
gcc -o monitorServer monitorServer.c citizen.c list.c monitorFunctions.c skipList.c -lpthread
./travelMonitorClient -m 5 -b 100 -c 5 -s 100 -i input_dir -t 2