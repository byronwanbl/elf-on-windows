all: sample1 sample2

sample1: sample1.c
	gcc -Wall -Wextra -fno-stack-protector -o sample1 sample1.c

sample2: sample2.cpp
	g++ -Wall -Wextra -fno-stack-protector -o sample2 sample2.cpp

.PHONY: all