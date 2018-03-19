all: furthest-seq
	
furthest-seq: src/definitions.h src/furthest.c src/test.c src/seq.c
	mpicc -DFURTHEST -DSEQUENTIAL -DVERBOSE src/test.c src/seq.c src/furthest.c -o furthest-seq

clean:
	rm furthest-seq

.PHONY: clean all

