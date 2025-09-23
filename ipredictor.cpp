// $Id: ipredictor.cpp,v 1.2 2005/12/30 21:03:55 kjamroz Exp $
#include "uid.h"
#include "ipredictor.h"
#include "clpcpredictor.h"
#include "cfirpredictor.h"
#include "cfftpredictor.h"
#include "cwaveletpredictor.h"


IPredictor *IPredictor::CreateInstance(char uid) 
{
	switch(uid)
	{
		case CSIMPLECOMPRESSEDFRAME_UID:
			return new CSimplePredictor();
		case CLPCRICEFRAME_UID:
			return new CLPCPredictor();
		case CFIRCOMPRESSEDFRAME_UID:
			return new CFIRPredictor();
		case CWAVELETCOMPRESSEDFRAME_UID:
			return new CWaveletPredictor();
		case CFFTCOMPRESSEDFRAME_UID:
			return new CFFTPredictor();
		default:
			return NULL;
	}
}

