# Makefile for new NiCad tools
# J.R. Cordy March 2010

MACHINESIZE = -m32
# MACHINESIZE = -m64 -mcmodel=medium
TURINGSIZE = -mLINUX -DHUGE

all : cloneclasses.x clonepairs.x crossclones.x getsource.x getnormsource.x tohtmlpairs.x streamprocess.x splitclasses.x

clonepairs.x: TLI.c TLS.c TLglob.h LINUX/cinterface main.c clonepairs.c
	gcc -w -O $(MACHINESIZE) -I . -o clonepairs.x main.c TLI.c TLS.c clonepairs.c

cloneclasses.x: TLI.c TLS.c TLglob.h LINUX/cinterface main.c cloneclasses.c
	gcc -w -O $(MACHINESIZE) -I . -o cloneclasses.x main.c TLI.c TLS.c cloneclasses.c

crossclones.x: TLI.c TLS.c TLglob.h LINUX/cinterface main.c crossclones.c
	gcc -w -O $(MACHINESIZE) -I . -o crossclones.x main.c TLI.c TLS.c crossclones.c

getsource.x: TLI.c TLS.c TLglob.h LINUX/cinterface main.c getsource.c
	gcc -w -O $(MACHINESIZE) -I . -o getsource.x main.c TLI.c TLS.c getsource.c

getnormsource.x: TLI.c TLS.c TLglob.h LINUX/cinterface main.c getnormsource.c
	gcc -w -O $(MACHINESIZE) -I . -o getnormsource.x main.c TLI.c TLS.c getnormsource.c

tohtmlpairs.x: TLI.c TLS.c TLglob.h LINUX/cinterface main.c tohtmlpairs.c
	gcc -w -O $(MACHINESIZE) -I . -o tohtmlpairs.x main.c TLI.c TLS.c tohtmlpairs.c

streamprocess.x: TLI.c TLS.c TLglob.h LINUX/cinterface main.c streamprocess.c
	gcc -w -O $(MACHINESIZE) -I . -o streamprocess.x main.c TLI.c TLS.c streamprocess.c

splitclasses.x: TLI.c TLS.c TLglob.h LINUX/cinterface main.c splitclasses.c
	gcc -w -O $(MACHINESIZE) -I . -o splitclasses.x main.c TLI.c TLS.c splitclasses.c

cloneclasses.c: cloneclasses.t linetable.i
	tpc $(TURINGSIZE) -O -A -q4 cloneclasses.t

clonepairs.c: clonepairs.t linetable.i
	tpc $(TURINGSIZE) -O -A -q4 clonepairs.t

crossclones.c: crossclones.t linetable.i
	tpc $(TURINGSIZE) -O -A -q4 crossclones.t

getsource.c: getsource.t 
	tpc $(TURINGSIZE) -O -A -q4 getsource.t

getnormsource.c: getnormsource.t 
	tpc $(TURINGSIZE) -O -A -q4 getnormsource.t

tohtmlpairs.c: tohtmlpairs.t 
	tpc $(TURINGSIZE) -O -A -q4 tohtmlpairs.t

streamprocess.c: streamprocess.t 
	tpc $(TURINGSIZE) -O -A -q4 streamprocess.t

splitclasses.c: splitclasses.t 
	tpc $(TURINGSIZE) -O -A -q4 splitclasses.t

clean:
	/bin/rm -f clonepairs.x cloneclasses.x crossclones.x getsource.x getnormsource.x tohtmlpairs.x streamprocess.x splitclasses.x

reset:
	@echo ""
	@echo "*** Don't do this unless you are CERTAIN that you know what you are doing! ***"
	@echo ""
	@echo "Press return to proceed, CTRL-c to abort: "
	@read x
	/bin/rm -f clonepairs.x clonepairs.c cloneclasses.x cloneclasses.c crossclones.x crossclones.c getsource.x getsource.c getnormsource.x getnormsource.c tohtmlpairs.x tohtmlpairs.c streamprocess.x streamprocess.c splitclasses.x splitclasses.c *.o
