all: furthest-seq
	
diag-seq: src/definitions.h src/diagonals.c src/seq.c src/test.c
	mpicc -DDIAGONAL -DSEQUENTIAL -DVERBOSE src/test.c src/seq.c src/diagonals.c -o diag-seq

furthest-seq: src/definitions.h src/furthest.c src/test.c src/seq.c
	mpicc -DFURTHEST -DSEQUENTIAL -DVERBOSE src/test.c src/seq.c src/furthest.c -o furthest-seq

diag-flood: src/definitions.h src/diagonals.c src/test.c src/flood.c
	mpicc -DDIAGONAL -DFLOOD -DVERBOSE src/test.c src/flood.c src/diagonals.c -o diag-flood

furthest-flood: src/definitions.h src/furthest.c src/test.c src/flood.c
	mpicc -DFURTHEST -DFLOOD -DVERBOSE src/test.c src/flood.c src/furthest.c -o furthest-flood

clean:
	rm furthest-seq

.PHONY: clean all

