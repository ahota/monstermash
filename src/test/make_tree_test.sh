#!/bin/bash

ID=0

if [ -f tree_test.txt ]; then
		rm tree_test.txt;
fi

echo "mkdir dir0" >> tree_test.txt
echo "cd dir0" >> tree_test.txt
ID=$(($ID + 1))
for i in {1..5}; do
		echo "mkdir dir$i" >> tree_test.txt
		echo "cd dir$i" >> tree_test.txt
		ID=$(($ID + 1))
		for j in $(seq 1 $i); do
				echo "mkdir dir$i-$j" >> tree_test.txt
				echo "cd dir$i-$j" >> tree_test.txt
				ID=$(($ID + 1))
				for k in $(seq 1 $j); do
						echo "mkdir dir$i-$j-$k" >> tree_test.txt
						echo "cd dir$i-$j-$k" >> tree_test.txt
						ID=$(($ID + 1))
						for l in $(seq 1 $k); do
								echo "open file$l w" >> tree_test.txt
								ID=$(($ID + 1))
								for m in $(seq 1 $ID); do
										echo "write $ID Oh hello, I didn't see you there. Don't mind me, I'm just testing my script here..." >> tree_test.txt
								done;
								echo "close file$l" >> tree_test.txt
						done;
						echo "cd .." >> tree_test.txt
				done;
				echo "cd .." >> tree_test.txt
		done;
		echo "cd .." >> tree_test.txt
done;
echo "cd .." >> tree_test.txt
echo "tree" >> tree_test.txt
echo "exit" >> tree_test.txt

