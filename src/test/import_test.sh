#!/bin/bash

DD_INPUT=/dev/urandom
DD_OUTPUT=/home/ahota/workspace/test/random.dat
MMASH_DIR=/home/ahota/workspace/monstermash/src/


for i in {1..4096}; do
		dd if=$DD_INPUT of=$DD_OUTPUT bs=1024 count=$i;
		{ time $MMASH_DIR/mmash < $MMASH_DIR/test/import_commands.txt > /dev/null; } |& grep real | awk '{print $2}' >> output.log;
done;

./mmash_import.py
