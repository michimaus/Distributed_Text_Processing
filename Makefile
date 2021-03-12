
.PHONY: clear

default: build

build:
	mpic++ tema3.cpp -o main

# build:
# 	mpic++ tema3.cpp -o main -pthread -Wall -Wextra

# run:
# 	mpirun -np 5 -oversubscribe ./main input5.txt

# run:
# 	mpirun -np 5 -oversubscribe ./main tema.in

run:
	mpirun -np 5 -oversubscribe ./main test.in

clear:
	rm main
