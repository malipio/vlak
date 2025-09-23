# $Id: Makefile,v 1.18 2005/12/30 21:03:55 kjamroz Exp $
all: tests wavdiff vlak

INCLUDES=-Ipacket/include -Ipacket/lossless/include -Ipacket/lossless/srcvlak

#CPPFLAGS=-g -Wall -I. $(INCLUDES)
CPPFLAGS=-O2 -march=pentium4 -Wall -I. $(INCLUDES)
#LDFLAGS=
CPP=g++

HDR:=$(wildcard *.h RiceCoder/*.h)
SRC:=$(wildcard *.cpp RiceCoder/*.cpp)
OBJ:=$(SRC:.cpp=.o)
WAVELETSRC:=$(wildcard packet/src/*.cpp packet/lossless/src/*.cpp packet/lossless/srcvlak/*.cpp)
WAVELETOBJ:=$(WAVELETSRC:.cpp=.o)
COMMONOBJ=File.o cblockinfo.o csimplecompressedframe.o cblock.o RiceCoder/cricecoder.o\
RiceCoder/cbitstream.o WaveFile.o BlocksProviderFromWaveReader.o \
streamops.o ichannelmanipulator.o icompressedframe.o cfircompressedframe.o \
clpcriceframe.o clpcpredictor.o ipredictor.o \
cfftcompressedframe.o cfftpredictor.o fft.o
TESTINCOBJ=testincludes.o $(COMMONOBJ) $(WAVELETOBJ)
TESTMAIN=testmain.o $(COMMONOBJ) BitVector.o $(WAVELETOBJ)
WAVDIFFOBJ=wavdiff.o WaveFile.o File.cpp cblock.o cblockinfo.o BlocksProviderFromWaveReader.o
VLAKOBJ=vlak.o $(COMMONOBJ) $(WAVELETOBJ)
tests: testincludes testmain testlpc


vlak: $(VLAKOBJ)
	$(CPP) -o vlak $(VLAKOBJ)

testlpc: testlpc.o $(COMMONOBJ) $(WAVELETOBJ)
	$(CPP) -o testlpc testlpc.o $(COMMONOBJ) $(WAVELETOBJ)
	
testincludes: $(TESTINCOBJ)
	$(CPP) -o testincludes $(TESTINCOBJ)

testmain: $(TESTMAIN)
	$(CPP) -o testmain $(TESTMAIN)

wavdiff: $(WAVDIFFOBJ)
	$(CPP) -o wavdiff $(WAVDIFFOBJ)

deps:
	$(CPP) $(CPPFLAGS) -MM $(SRC) $(WAVELETSRC) > deps

include deps

.cpp.o:
	$(CPP) -c $(CPPFLAGS) $< -o $@

clean:
	rm -f $(OBJ)
	rm -f $(WAVELETOBJ)
	rm -f testincludes
	rm -f testmain
	rm -f testlpc
	rm -f wavdiff
	rm -f vlak

mrproper: clean
	rm -f deps

ctags: 
	exuberant-ctags $(SRC) $(HDR)
