FLAGS=-O3 -fopenmp

CC=gcc

RM=rm -f

EXEC=dna

all: $(EXEC)

$(EXEC):
	$(CC) $(FLAGS) xpto.c -c -o dna.o
	$(CC) $(FLAGS) dna.o -o $(EXEC)

run:
	OMP_NUM_THREADS=24 ./$(EXEC)

clean:
	$(RM) dna.o $(EXEC)	
