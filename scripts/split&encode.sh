#!/usr/bin/env bash

if [ ! -f $1 ]; then
    printf "No such file: " "$1"
    exit 1
fi


#../build/gabacify encode -i $1
#
input_size="$(wc -c $1 | awk '{print $1}' | tail -1)"
#echo "Total Compressed File Size in one piece: $totalsize"
#rm $1*bytestream
#rm $1*json
max_num_blocks=1000
min_num_blocks=1
for b in 4096 16384 65536 262144 1048576 4194304 16777216; do
  (
      if [[(($input_size -lt $(($b*$max_num_blocks))))]] && [[(($input_size -gt $b))]]; then
          # split file
          suffix=".split"
          split -b $b -a 5 $1 "$1$suffix$b"
          # encode files
          task(){
            ../build/gabacify --task encode --input_file_path $f
          }
          nr_of_threads=7
          (
          for f in $1$suffix$b*; do
            ((i=i%nr_of_threads)); ((i++==0)) && wait
            task "$f" &
          done
          wait
          )
          # Print total file size
          size="$(wc -c $1$suffix$b*bytestream | awk '{print $1}' | tail -1)"
          echo "Total Compressed File Size With RA size $b: $size"

          # remove files
          rm $1$suffix$b*bytestream
          rm $1$suffix$b*json
          rm $1$suffix$b*
      fi
)
done
