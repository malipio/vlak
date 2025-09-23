// $Id: wavdiff.cpp,v 1.1 2005/11/18 01:05:58 wwasiak Exp $
#include "WaveFile.h"
#include "stdio.h"
#include "cblock.h"

#define BLOCKSIZE 1024

int main(int argc, char** argv)
{
if (argc!=3)
    {
	printf("Usage: %s file1.wav file2.wav\n",argv[0]);
	exit(0);
    }

CWaveReader reader1;
CWaveReader reader2;

//TODO sprawdzic czy plik jest bo krzysiek nie rzuca wyjatku tylko robi assert
//na razie to tryowanie nie ma sensu

try
    {
	reader1.Open(argv[1]);
    }
catch (...)
    {
	printf("Problem with file: '%s'\n",argv[1]);
	exit(1);
    }

try
    {
	reader2.Open(argv[2]);
    }
catch (...)
    {
	printf("Problem with file: '%s'\n",argv[2]);
	exit(1);
    }

if (reader1.ChannelsCount()!=2)
    {
	printf("Only 2 channel wav files are supported\n");
	exit(2);
    }

//sprawdzamy dlugosc
if (reader1.SamplesCount()!=reader2.SamplesCount())
    {
	printf("Files have different sizes\n");
	exit(3);
    }
else
    {
	//wlasciwe porownanie
	IBlocksProvider * provider1=reader1.GetBlocksProvider(BLOCKSIZE);
	IBlocksProvider * provider2=reader2.GetBlocksProvider(BLOCKSIZE);
	
	int offset=0;
	CBlock block1(BLOCKSIZE),block2(BLOCKSIZE);

	while (provider1->HasNext() && provider2->HasNext())
	    {
	    block1=provider1->GetNextBlock();
	    block2=provider2->GetNextBlock();
	    
	    for (int i=0; i<block1.GetBlockSize(); i++)
		{
		    if ( (block1.GetSamples(0)[i]!=block2.GetSamples(0)[i])
			|| (block1.GetSamples(1)[i]!=block2.GetSamples(1)[i]) )
			{
			    printf("%d : (%d,%d) (%d,%d)\n",offset+i,block1.GetSamples(0)[i],block1.GetSamples(1)[i],block2.GetSamples(0)[i],block2.GetSamples(1)[i]);
			}
		}
	    offset+=BLOCKSIZE;
	    }
    }

//pliki same sie zamkna
return 0;
}
