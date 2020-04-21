#!/bin/bash

# download test files
test -f book1.txt || wget https://www.gutenberg.org/files/1342/1342-0.txt -O book1.txt
test -f book2.txt || wget https://www.gutenberg.org/ebooks/1260.txt.utf-8 -O book2.txt
test -f book3.txt || wget https://www.gutenberg.org/ebooks/174.txt.utf-8 -O book3.txt
test -f book4.txt || wget https://www.gutenberg.org/ebooks/768.txt.utf-8 -O book4.txt
test -f book5.txt || wget https://www.gutenberg.org/files/2554/2554-0.txt -O book5.txt

g++ -std=c++17 -O2 -g ch01_problemv1.cpp -o problemv1.out && ./problemv1.out
g++ -std=c++17 -O2 -g ch01_problemv2.cpp -o problemv2.out && ./problemv2.out
g++ -std=c++17 -O2 -g ch01_problemv3.cpp -o problemv3.out && ./problemv3.out
