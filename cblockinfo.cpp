// $Id: cblockinfo.cpp,v 1.1 2005/11/13 01:11:32 wwasiak Exp $

#include "cblockinfo.h"

CBlockInfo::CBlockInfo(char bits,int freq,char chan)
{
	BitsPerSample=bits;
        SamplingFrequency=freq;
	NumOfChannels=chan;
}

char CBlockInfo::GetBitsPerSample() const
{
        return BitsPerSample;
}

int CBlockInfo::GetSamplingFrequency() const
{
        return SamplingFrequency;
}

char CBlockInfo::GetNumOfChannels() const
{
        return NumOfChannels;
}
