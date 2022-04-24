# Project:  neural chess
# Author:   Jakub Urbanek
# Year:     2022

CC = gcc
CFLAGS = -fopenmp -Wall -g -O3
LIBS= -lm

OBJFILES= main.o ai.o chess_net.o fcnn.o neuron.o chess_logic.o chess_structs.o

SRCDIR= src
BINDIR= bin
BINNAME= nn
POPULATION_SAVE_DIR= population

default: build clean

build: makedir $(OBJFILES)
	$(CC) $(CFLAGS) $(OBJFILES) $(LIBS) -o $(BINDIR)/$(BINNAME)

%.o : $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

makedir:
	rm -rf $(BINDIR)
	mkdir $(BINDIR)
	cd $(BINDIR); mkdir $(POPULATION_SAVE_DIR)

clean:
	rm -f $(OBJFILES)

run:
	cd $(BINDIR); ./$(BINNAME)

run-valgrind:
	cd $(BINDIR); valgrind --leak-check=full ./$(BINNAME)