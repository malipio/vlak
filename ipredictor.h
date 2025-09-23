// $Id: ipredictor.h,v 1.8 2005/12/28 19:30:10 pmalinow Exp $
#ifndef IPREDICTOR_H
#define IPREDICTOR_H
#include "iframe.h"
#include "cblock.h"
#include "csimplecompressedframe.h" // CSimpleCompressedFrame

// Interface IPredictor
// interfejs sluzacy do kodowania predykcyjnego blokow 
class IPredictor 
{
public:
	virtual ~IPredictor() {};
	
	virtual IFrame* EncodeBlock(const CBlock& block) = 0;
		
	virtual CBlock DecodeFrame(const IFrame& frame) = 0;
	static IPredictor *CreateInstance(char uid);
};

class IAdaptivePredictor : public IPredictor // TODO: przyda sie czy nie?
{
public:
	// podajemy zakres rzedow np. 1-8
	// a predyktor dopasuje najlepszy korzystajac z
	// obliczonego bledu predykcji
	virtual void SetMinOrder(char order) = 0;
	virtual void SetMaxOrder(char order) = 0;
	// TODO: czy przypadkiem nie sa potrzebne funkcje
	// zwracajace blad predykcji - dla adaptacyjnego compressora Rice'a
	// 1. dodac je tutaj
	// 2. dodac do arg metody EncodeBlock
	// 3. dodac do zwracanej ramki
};

class CSimplePredictor : public IPredictor
{
public:
	IFrame* EncodeBlock(const CBlock& block)
	{
		return new CSimpleCompressedFrame(block);		
		
	}
		
	virtual CBlock DecodeFrame(const IFrame& frame)
	{
		const CSimpleCompressedFrame& csFrame = 
			dynamic_cast<const CSimpleCompressedFrame&>(frame);
		CBlock block(csFrame.GetInfo(),csFrame.GetBlockSize());
		block.SetManipulatorUID(csFrame.GetGUID().GetManipulatorUID());
		
		for(int ch = 0; ch < frame.GetInfo().GetNumOfChannels(); ++ch)
		{
			CSample *ptr = csFrame.GetSamples(ch);
			for(int i = 0; i < block.GetBlockSize(); ++i)
				block.SetSample(ch,i,ptr[i]);
		}
		return block;
	}

};
#endif //IPREDICTOR_H

