// $Id: cblock.cpp,v 1.10 2005/11/26 19:10:57 pmalinow Exp $

#include "cblock.h"
//#include "null.h"
#include <iostream>

//ustanawia rozmiar bloka - bedzie on staly
CBlock::CBlock(int Size, char bits, int freq, char chan) : MyInfo(bits,freq,chan),
					   manipUID(CSIMPLECHANNELMANIPULATOR_UID)
{
BlockSize=Size;
if (BlockSize>0)
	Data=new CSample[Size*chan];
    else
	Data=NULL;
}

CBlock::CBlock(const IInfo& info,int blockSize) : BlockSize(blockSize), MyInfo(info),
					   manipUID(CSIMPLECHANNELMANIPULATOR_UID)
{
if (BlockSize>0)
	Data = new CSample[BlockSize*MyInfo.GetNumOfChannels()];
    else
	Data=NULL;
}

CBlock::CBlock(const CBlock & orig) : BlockSize(orig.BlockSize), MyInfo(orig.MyInfo),
				      manipUID(orig.manipUID)
{
if (orig.BlockSize>0)
	Data=new CSample[orig.BlockSize*orig.MyInfo.GetNumOfChannels()];
    else
	Data=NULL;
for (int i=0; i<orig.BlockSize*orig.MyInfo.GetNumOfChannels(); i++) Data[i]=orig.Data[i];
}

CBlock::~CBlock()
{
//TODO w gcc 4.0 nie dziala nie wiadomo dlaczego
if (Data!=NULL)
    delete[] Data;
Data=NULL;
}

CBlock CBlock::operator=(const CBlock & orig)
{
if (Data!=NULL)
    delete[] Data;
BlockSize=orig.BlockSize;
MyInfo=orig.MyInfo;
manipUID = orig.manipUID;
Data=new CSample[BlockSize*MyInfo.GetNumOfChannels()];
for (int i=0; i<BlockSize*MyInfo.GetNumOfChannels(); i++) Data[i]=orig.Data[i];
return orig;
}

CSample* CBlock::GetSamples(int chNum) const
{
return &(Data[chNum*BlockSize]);
}

const IInfo& CBlock::GetInfo() const
{
return MyInfo;
}

// zwraca rozmiar bloku *w probkach* na kanal
int CBlock::GetBlockSize() const
{
return BlockSize;
}
