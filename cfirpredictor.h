// $Id: cfirpredictor.h,v 1.3 2005/12/04 16:45:50 wwasiak Exp $
#ifndef IFIRPREDICTOR_H
#define IFIRPREDICTOR_H
#include "iframe.h"
#include "cblock.h"
#include "ipredictor.h"
//#include "csimplecompressedframe.h" // CSimpleCompressedFrame
#include "cfircompressedframe.h" // CFIRCompressedFrame


class CFIRPredictor : public IPredictor
{
public:
	IFrame* EncodeBlock(const CBlock& block) //robi new - > potem trzeba to deletowac
	{ //TODO na razie fir dziala tak jak simple
		CFIRCompressedFrame * result=new CFIRCompressedFrame(block);
		//na razie spelnia kryteria iframe
		//teraz zaczynamy robic z niej compressedframe
		for (int i=0; i < block.GetInfo().GetNumOfChannels(); i++)
		    { //przepisujemy pierwsze sample do startsamples
		    (result->startsamples)[0+3*i]=result->samples[0+block.GetBlockSize()*i];
		    (result->startsamples)[1+3*i]=result->samples[1+block.GetBlockSize()*i];
		    (result->startsamples)[2+3*i]=result->samples[2+block.GetBlockSize()*i];
		    }

		//liczymy rozmiar nowego data
		//        rozmiar probki * 2 | bez 3 pierwszych sampli    |  ilosc kanalow
		int size = sizeof(CExtSample) * (block.GetBlockSize()-3) * block.GetInfo().GetNumOfChannels();
		CBitStream resultdata(size);

		for (int i=0; i < block.GetInfo().GetNumOfChannels(); i++)
		    {
		    int xnpredicted, xnerror;
		    for (int n=3; n < block.GetBlockSize(); n++)
			{
			//x pred = 3x[n-1] - 3x[n-2] + x[n-3]
			xnpredicted = 3*result->samples[(n-1)+block.GetBlockSize()*i]
					- 3*result->samples[(n-2)+block.GetBlockSize()*i]
					+ result->samples[(n-3)+block.GetBlockSize()*i];
			xnerror=result->samples[n+block.GetBlockSize()*i] - xnpredicted;
			resultdata.Write((byte *)&xnerror,sizeof(CExtSample));
			}
		    }

		//przepisujemy dane do data
		resultdata.Reset();
		result->SetDataStream(resultdata);
		return result; //TODO juz z predykcja ale nie zakodowana ricem
	}
		
	virtual CBlock DecodeFrame(const IFrame& frame)
	{
		const CFIRCompressedFrame& csFrame = 
			dynamic_cast<const CFIRCompressedFrame&>(frame);
		CBlock block(csFrame.GetInfo(),csFrame.GetBlockSize());
		block.SetManipulatorUID(csFrame.GetGUID().GetManipulatorUID());

		CBitStream newdata(*csFrame.data,true); //TODO cala kopia - muzse robic bo tamto jest const
		newdata.Reset();
		for(int ch = 0; ch < frame.GetInfo().GetNumOfChannels(); ++ch)
		{
		    //przepisanie 3 sampli startowych
		    block.GetSamples(0)[0+block.GetBlockSize()*ch]=csFrame.startsamples[0+3*ch];
		    block.GetSamples(0)[1+block.GetBlockSize()*ch]=csFrame.startsamples[1+3*ch];
		    block.GetSamples(0)[2+block.GetBlockSize()*ch]=csFrame.startsamples[2+3*ch];
		    
		    int xnpredicted,xnerror;
		    for (int n=3; n < block.GetBlockSize(); n++)
			{
			//x pred = 3x[n-1] - 3x[n-2] + x[n-3]
			xnpredicted = 3*block.GetSamples(0)[(n-1)+block.GetBlockSize()*ch]
					- 3*block.GetSamples(0)[(n-2)+block.GetBlockSize()*ch]
					+ block.GetSamples(0)[(n-3)+block.GetBlockSize()*ch];
			newdata.GetNextNBytes((byte *) &xnerror, sizeof(CExtSample));
			block.GetSamples(0)[n+block.GetBlockSize()*ch]=(CSample)(xnpredicted+xnerror); //TODO 
			}
		}
		return block;
	}

};

#endif //IFIRPREDICTOR_H
