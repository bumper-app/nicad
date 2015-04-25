# Initial make of NiCad for a new platform

all:
	cd tools; make clean; make; cd ..
	cd txl; make clean; make; cd ..

clean:
	cd tools; make clean; cd ..
	cd txl; make clean; cd ..
