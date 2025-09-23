// $Id: cblockinfo.h,v 1.2 2005/11/13 16:56:13 pmalinow Exp $
#ifndef CBLOCKIINFO_H
#define CBLOCKIINFO_H

#include "iinfo.h"

class CBlockInfo : public IInfo
{
public:
	CBlockInfo(char,int,char); // pkp. a gdzie nazwy argumentow? :P
	CBlockInfo(const IInfo& info)
	{
		BitsPerSample = info.GetBitsPerSample();
		SamplingFrequency = info.GetSamplingFrequency();
		NumOfChannels = info.GetNumOfChannels();
	}

        virtual char GetBitsPerSample() const;
        virtual int GetSamplingFrequency() const;
        virtual char GetNumOfChannels() const;
private:
        char BitsPerSample;
        int SamplingFrequency;
        char NumOfChannels;
};
#endif //CBLOCKIINFO_H
