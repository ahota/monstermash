echo "open file w" > huge_file.txt
for i in {1..100000}; do
    echo "write 1 Lorem ipsum " >> huge_file.txt;
done;
echo "close file" >> huge_file.txt
echo "stat file" >> huge_file.txt
echo "exit" >> huge_file.txt

