# Project:  neural chess
# Author:   Jakub Urbanek
# Year:     2022

CC = gcc
CFLAGS = -Wall -g -O3
LIBS= -lm

OBJFILES= main.o chess_net.o fcnn.o neuron.o

SRCDIR= src
BINDIR= bin
BINNAME= nn

default: build clean

build: makedir $(OBJFILES)
	$(CC) $(CFLAGS) $(OBJFILES) $(LIBS) -o $(BINDIR)/$(BINNAME)

%.o : $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

makedir:
	rm -rf $(BINDIR)
	mkdir $(BINDIR)

clean:
	rm -f $(OBJFILES)

run:
	cd $(BINDIR); ./$(BINNAME)

run-valgrind:
	cd $(BINDIR); valgrind --leak-check=full ./$(BINNAME)