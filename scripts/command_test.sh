#!/bin/bash

time=1681741414

for i in {1..99}
do
	echo "$i,$time,18,19,20,30,41,52" > ./command/real_cmd_99$i
	time=`expr $time + 10`
	sleep 1
done
