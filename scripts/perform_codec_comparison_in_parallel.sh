#!/usr/bin/env bash

max_nr_of_threads=$2
current_nr_of_threads=0
for f in $1*; do 
	read current_nr_of_threads <<< $(pidof time | wc -w | awk '{print $1}')
	while [ $current_nr_of_threads -ge $max_nr_of_threads ]
		do
		echo Running $current_nr_of_threads threads. Maximum was set to $max_nr_of_threads.
	      	sleep 60
		read current_nr_of_threads <<< $(pidof time | wc -w | awk '{print $1}')
		done
	./perform_codec_comparison.sh $f >$f.measurements &
        sleep 5 #to avoid massive number of threads during parallel startup of multiple occurrences of the script
done
while true
	do
	read current_nr_of_threads <<< $(pidof time | wc -w | awk '{print $1}')
	echo Running $current_nr_of_threads threads. Maximum was set to $max_nr_of_threads.
	sleep 60
	done
wait
