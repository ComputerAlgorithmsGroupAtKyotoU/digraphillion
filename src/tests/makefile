PAR_DIR=..
SAPPORO_DIR=../SAPPOROBDD
DIGRAPHILLION_DIR=../digraphillion
OPT = -O3 -DB_64 -I.

all: test_degree

test_degree: test_degree_constraints.cpp
	g++ -std=c++14 test_degree_constraints.cpp ../digraphillion/graphset.cc -o test_degree -I$(DIGRAPHILLION_DIR) -I$(PAR_DIR) -I$(SAPPORO_DIR)

../SAPPOROBDD/bddc.o: ../SAPPOROBDD/bddc.c ../SAPPOROBDD/bddc.h
	gcc $(OPT) ../SAPPOROBDD/bddc.c -c -o ../SAPPOROBDD/bddc.o

../SAPPOROBDD/BDD.o: ../SAPPOROBDD/BDD.cc ../SAPPOROBDD/BDD.h
	g++ $(OPT) ../SAPPOROBDD/BDD.cc -c -o ../SAPPOROBDD/BDD.o

../SAPPOROBDD/ZBDD.o: ../SAPPOROBDD/ZBDD.cc ../SAPPOROBDD/ZBDD.h
	g++ $(OPT) ../SAPPOROBDD/ZBDD.cc -c -o ../SAPPOROBDD/ZBDD.o

clean:
	rm -f test_degree test_degree.exe *.o ../SAPPOROBDD/bddc.o ../SAPPOROBDD/BDD.o ../SAPPOROBDD/ZBDD.o
