// $Id: clpcpredictor.h,v 1.4 2005/11/25 22:55:51 pmalinow Exp $
#ifndef CLPCPREDICTOR_H
#define CLPCPREDICTOR_H
#include <assert.h>
#include "ipredictor.h"
#include "iframe.h"
#include "cblock.h"
#include "clpcriceframe.h" // CLPCRiceFrame
#include "streamops.h"

// Implementacja klasy predyktora opartego na LPC
// (wykorzystuje CLPCRiceFrame)
class CLPCPredictor : public IAdaptivePredictor
{
protected:
	char minOrder;
	char maxOrder;

	// funkcja oblicza autokorelacje sygnalu
	// maxLag == rzad predyktora + 1 ??
	// r[] - tablica w ktorej zapiszemy wyniki (musi byc typu double)
	void ComputeAutocorrelation(CSample samples[],int samplesCount,
			int maxLag, double r[]);

	void ComputeResidual(CLPCRiceFrame& lpcFrame, const CBlock& block);

	void ReproduceSamples(CLPCRiceFrame& lpcFrame, CBlock& block);

	void ComputeCoefficients(int chNum,double r[], float* c[], double error[]);

	// libFLAC
	void FLAC__lpc_compute_lp_coefficients(const double autoc[], unsigned max_order, 
		float *lp_coeff[] ,double error[]);
	
	int SelectBestOrder(float *c[],double error[],char maxOrder,int blockSize);
	
public:
	virtual ~CLPCPredictor() {}
	
	CLPCPredictor() : minOrder(0), maxOrder(0) {}
	
	virtual void SetMinOrder(char order);
	
	virtual void SetMaxOrder(char order);

	// pomocnicza funkcyjka
	void SetOrder(int order);
	
	virtual IFrame* EncodeBlock(const CBlock& block);
		
	virtual CBlock DecodeFrame(const IFrame& frame);
};

#endif //CLPCPREDICTOR_H

