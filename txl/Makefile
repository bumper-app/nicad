# Makefile to compile all TXL tools used in NiCad

.SUFFIXES:
.SUFFIXES:.txl .x

SRCS = $(wildcard *.txl)
FILTEREDSRCS = $(filter-out generic-%.txl,$(SRCS))
OBJS = $(patsubst %.txl,%.x,$(FILTEREDSRCS))   

all: $(OBJS)

%.x: %.txl
	txlc -s 400 $<

clean:
	/bin/rm -f *.x

