#!/bin/bash

echo "Download ***************************************************************"
test -f book1.txt || wget https://www.gutenberg.org/files/1342/1342-0.txt -O book1.txt
test -f book2.txt || wget https://www.gutenberg.org/ebooks/1260.txt.utf-8 -O book2.txt
test -f book3.txt || wget https://www.gutenberg.org/ebooks/174.txt.utf-8 -O book3.txt
test -f book4.txt || wget https://www.gutenberg.org/ebooks/768.txt.utf-8 -O book4.txt
test -f book5.txt || wget https://www.gutenberg.org/files/2554/2554-0.txt -O book5.txt

echo "Basic ******************************************************************"
g++ -std=c++17 -O2 -g ch01_problemv1.cpp -o problemv1.out && ./problemv1.out
g++ -std=c++17 -O2 -g ch01_problemv2.cpp -o problemv2.out && ./problemv2.out
g++ -std=c++17 -O2 -g ch01_problemv3.cpp -o problemv3.out && ./problemv3.out


echo "Profiling **************************************************************"

export TIME="%e,%U,%S,%M" # real, user, sys, max-mem

g++ -std=c++17 -O2 -g ch01_perf.cpp -o perf_imperative.out
echo "param,real,user,sys,maxmem" > perf_imperative.csv
for param in 1000 10000 100000 1000000 10000000; do
    echo -n "$param," >> perf_imperative.csv
    /usr/bin/time ./perf_imperative.out 0 $param 2> >(tee -a  perf_imperative.csv)
done

g++ -std=c++17 -O2 -g -DFUNCTIONAL_FLAG ch01_perf.cpp -o perf_functional.out
echo "param,real,user,sys,maxmem" > perf_functional.csv
for param in 1000 10000 100000 1000000 10000000; do
    echo -n "$param," >> perf_functional.csv
    /usr/bin/time ./perf_functional.out 0 $param 2> >(tee -a  perf_functional.csv)
done

./analyse_time.py --name perf *.csv
./analyse_time.py --logx --logy --name perflogscale *.csv

mprof run ./perf_functional.out 0 10000000
mprof plot -o perfmem.png


echo "Cleanup ****************************************************************"

rm *.out
rm *.dat
#rm *.csv
