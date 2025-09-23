
#
# Makefile for Wavelet Packet code
#
# This makefile assumes that the Microsoft environment has
# been enhanced with the cygwin UNIX utilities for Win32.
# These are available from Red Hat (which purchased Cygnus).
#
# Command:
#    nmake -f Makefile
#

DEBUG     = -Zi
BROWSE    = -FR

CFLAGS    = $(BROWSE) $(DEBUG) -DWIN32 -Tp
OBJ       = obj
EXE       = .exe

LOCALINC = include

DOXYPATH = e:\doxygen\bin

INCLUDE = -I$(LOCALINC) -I "d:\Program Files\Microsoft Visual Studio\Vc98\Include"

PACKET_OBJ = blockpool.$(OBJ) packtree_base.$(OBJ) packtree.$(OBJ) packfreq.$(OBJ) costshannon.$(OBJ) invpacktree.$(OBJ) 

all: packtest freqtest


packtest: packtest$(EXE) 


freqtest: freqtest$(EXE)


packtest$(EXE): $(PACKET_OBJ) packtest.$(OBJ) 
	$(CC) $(PACKET_OBJ) packtest.$(OBJ) $(DEBUG) -o packtest$(EXE)


freqtest$(EXE): $(PACKET_OBJ) freqtest.$(OBJ) 
	$(CC) $(PACKET_OBJ) freqtest.$(OBJ) $(DEBUG) -o freqtest$(EXE)

#
# clean-up for Microsoft object and browser files and emacs temps
#
clean:
	rm -f *.obj *.pdb *.sbr *.ilk *.exe 
	rm -f include/*~
	rm -f src/*~

doxygen:
	$(DOXYPATH)\doxygen doxygenDocConfig

include\costbase.h: include\packnode.h

include\costshannon.h: include\costbase.h include\packnode.h

include\costthresh.h: include\costbase.h include\packnode.h

include\haar.h: include\liftbase.h

include\haar_classic.h: include\liftbase.h

include\line.h: include\liftbase.h

include\packdata.h: include\blockpool.h

include\invpacktree.h: include\packdata.h

include\packnode.h: include\packdata.h

include\packdata_list.h: include\fifo_list.h include\packdata.h

include\packcontainer.h: include\packnode.h

include\queue.h: include\fifo_list.h include\packnode.h

include\packtree_base.h: include\packnode.h include\liftbase.h

include\packtree.h: include\packtree_base.h include\packcontainer.h include\liftbase.h

include\packfreq.h: include\packtree_base.h include\liftbase.h

include\invpacktree.h: include\liftbase.h include\list.h include\packcontainer.h include\packdata.h include\packdata_list.h

packtree_base.$(OBJ): src\$*.cpp include\$*.h
	$(CC) -c $(INCLUDE) $(CFLAGS) src\$*.cpp


packtree.$(OBJ): src\$*.cpp include\$*.h include\queue.h include\packcontainer.h include\blockpool.h
	$(CC) -c $(INCLUDE) $(CFLAGS) src\$*.cpp


packfreq.$(OBJ): src\$*.cpp include\$*.h
	$(CC) -c $(INCLUDE) $(CFLAGS) src\$*.cpp


costshannon.$(OBJ): src\$*.cpp include\$*.h
	$(CC) -c $(INCLUDE) $(CFLAGS) src\$*.cpp

invpacktree.$(OBJ): src\$*.cpp include\$*.h include\blockpool.h
	$(CC) -c $(INCLUDE) $(CFLAGS) src\$*.cpp

blockpool.$(OBJ): src\$*.cpp include\$*.h
	$(CC) -c $(INCLUDE) $(CFLAGS) src\$*.cpp

local_new.$(OBJ): src\$*.cpp
	$(CC) -c $(INCLUDE) $(CFLAGS) src\$*.cpp


packtest.$(OBJ): src\$*.cpp include\haar.h include\haar_classic.h include\packnode.h include\packcontainer.h include\packtree.h include\invpacktree.h include\costshannon.h include\costthresh.h
	$(CC) -c $(INCLUDE) $(CFLAGS) src\$*.cpp


freqtest.$(OBJ): src\$*.cpp include\haar_classicFreq.h include\packcontainer.h
	$(CC) -c $(INCLUDE) $(CFLAGS) src\$*.cpp
