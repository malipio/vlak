
#
# Makefile for lossless wavelet compression software
#
# This makefile assumes that the Microsoft environment has
# been enhanced with the cygwin UNIX utilities for Win32.
# These are available from Red Hat (which purchased Cygnus).
#
# Command:
#    nmake -f Makefile
#

#CC	  = g++-3.4
CC	  = g++-3.3
#CC	  = g++
DEBUG     = -g
BROWSE    =

#GNU_FLAGS = -Wno-deprecated -Wall
GNU_FLAGS = -Wall
CFLAGS    = $(BROWSE) $(DEBUG)
OBJ       = o
EXE       = .exe

TOPINC = ../include

DOXYPATH = e:/doxygen/bin

INCLUDE = -Iinclude -I$(TOPINC) -I../data/include

PACKET_OBJ = blockpool.$(OBJ) packtree_base_int.$(OBJ) packtree_int.$(OBJ) invpacktree_int.$(OBJ) costwidth.$(OBJ) support.$(OBJ) yahooTS.$(OBJ)

all: compresstest$(EXE)


compresstest: compresstest$(EXE) 


compresstest$(EXE): $(PACKET_OBJ) compresstest.$(OBJ) 
	$(CC) $(PACKET_OBJ) compresstest.$(OBJ) $(DEBUG) -o compresstest$(EXE)

#
# clean-up for Microsoft object and browser files and emacs temps
#
clean:
	rm -f *.obj *.pdb *.sbr *.ilk *.exe *.o
	rm -f include/*~
	rm -f src/*~


$(TOPINC)/costbase.h: $(TOPINC)/packnode.h

include/costwidth.h: $(TOPINC)/costbase.h

include/line_int.h: $(TOPINC)/liftbase.h

$(TOPINC)/packdata.h: $(TOPINC)/blockpool.h

include/invpacktree_int.h: $(TOPINC)/packdata.h

$(TOPINC)/packnode.h: $(TOPINC)/packdata.h

$(TOPINC)/packdata_list.h: $(TOPINC)/fifo_list.h $(TOPINC)/packdata.h

$(TOPINC)/packcontainer.h: $(TOPINC)/packnode.h

$(TOPINC)/queue.h: $(TOPINC)/fifo_list.h $(TOPINC)/packnode.h

include/packtree_base_int.h: $(TOPINC)/packnode.h $(TOPINC)/liftbase.h

include/packtree_int.h: include/packtree_base_int.h $(TOPINC)/packcontainer.h $(TOPINC)/liftbase.h

include/invpacktree_int.h: $(TOPINC)/liftbase.h $(TOPINC)/list.h $(TOPINC)/packcontainer.h $(TOPINC)/packdata.h $(TOPINC)/packdata_list.h

packtree_base_int.$(OBJ):
	$(CC) -c $(INCLUDE) $(CFLAGS) src/$*.cpp


packtree_int.$(OBJ): $(TOPINC)/queue.h $(TOPINC)/packcontainer.h $(TOPINC)/blockpool.h
	$(CC) -c $(INCLUDE) $(CFLAGS) src/$*.cpp


invpacktree_int.$(OBJ): $(TOPINC)/blockpool.h
	$(CC) -c $(INCLUDE) $(CFLAGS) src/$*.cpp

blockpool.$(OBJ): 
	$(CC) -c $(INCLUDE) $(CFLAGS) ../src/$*.cpp

costwidth.$(OBJ): 
	$(CC) -c $(INCLUDE) $(CFLAGS) src/$*.cpp

support.$(OBJ): 
	$(CC) -c $(INCLUDE) $(CFLAGS) src/$*.cpp

yahooTS.$(OBJ): 
	$(CC) -c $(INCLUDE) $(CFLAGS) ../data/src/$*.cpp

compresstest.$(OBJ): include/line_int.h $(TOPINC)/packnode.h $(TOPINC)/packcontainer.h include/packtree_int.h include/invpacktree_int.h 
	$(CC) -c $(INCLUDE) $(CFLAGS) src/$*.cpp

%.o: src/%.cpp include/%.h
%.o: ../src/%.cpp ../include/%.h
