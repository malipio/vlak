// $Id: cfftpredictor.h,v 1.6 2006/01/12 03:04:05 kjamroz Exp $
#ifndef IFFTPREDICTOR_H
#define IFFTPREDICTOR_H

#include <assert.h>

#include "iframe.h"
#include "cblock.h"
#include "ipredictor.h"

#include "fft.h"


class CFFTPredictor : public IPredictor
{

public:  //from IPredictor
	virtual IFrame* EncodeBlock(const CBlock& block); //robi new - > potem trzeba to deletowac
	virtual CBlock DecodeFrame(const IFrame& frame);

public:
	CFFTPredictor()
	{
		QFACTOR = 8.0f;
		MINRELAMPL = 0.01f;
		MINRELAMPL2 = MINRELAMPL*MINRELAMPL; 
	}

	void SetParameters(float qfactor, float minrelampl)
	{
		//Qfactor can be only integer
		QFACTOR=(int)qfactor;
		MINRELAMPL=minrelampl;
		MINRELAMPL2 = MINRELAMPL*MINRELAMPL;
	}

private:
	float QFACTOR;
	float MINRELAMPL;
	float MINRELAMPL2; 

	CBitStream* CompressQuantizedSpectrum(int blockSize, int* re, int* im);

	void DecompressQuantizedSpectrum(CBitStream* stream, int blockSize, int* re, int* im);

	CFFT iFFT;
};

#endif //IFFTPREDICTOR_H
