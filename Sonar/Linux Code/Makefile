EXE = RecordData

CC = gcc
CXX = g++

SDIR = ./src
IDIR = ./inc
CFLAGS=-I$(IDIR)
CXFLAGS=-I$(IDIR)

ODIR=obj
LDIR =./lib

LIBS=-lm

_OBJ = RecordData.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SDIR)/%.cpp $(IDIR)/RecordData.h
	$(CXX) -c -o $@ $< $(CXFLAGS)

RecordData: $(OBJ)
	$(CXX) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: all
all: $(EXE) 

.PHONY: install
install:
	@echo Not currently implemented.  

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core $(EXE) $(INCDIR)/*~ 
