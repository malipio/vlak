#include <iomanip>
#include <sstream>
#include "streamops.h"
using namespace std;

// pomocniczne nazwy

char *predictorNames[] = { "Simple",
		"LPC",
		"FIR", 
		"Wavelet",
		"FFT",
		NULL};
#define MAX_P_IDX 5

char *frameNames[] = { "CSimpleCompressedFrame",
		"CLPCRiceFrame",
		"CFIRCompressedFrame", 
		"CWaveletCompressedFrame",
		"CFFTCompressedFrame",
		NULL};

char *manipNames[] = { "CSimpleChannelManipulator",
		"CMidSideChannelManipulator",
		"CAdaptiveChannelManipulator",
		NULL };
#define MAX_M_IDX 3

char *coderNames[] = { "CRiceCoder", "CSimpleCompressor", NULL};

char * predictorName(char uid) { return uid < MAX_P_IDX && uid >= 0?predictorNames[(int)uid]:NULL; }
char * frameName(char uid) { return frameNames[(int)uid]; }
char * manipName(char uid) { return uid < MAX_M_IDX && uid >= 0?manipNames[(int)uid]:NULL; }
char * coderName(char uid) { return coderNames[(int)uid]; }

ostream& operator<<(ostream& ostr, const IInfo& info)
{
	return info.Print(ostr);
}

ostream& operator<<(ostream& ostr, const IFrame& frame)
{
	return frame.Print(ostr);
}

ostream& operator<<(ostream& ostr, const CBitStream& bitStream)
{
	return bitStream.Print(ostr);
}

ostream& operator<<(ostream& ostr, const GUID& guid)
{
	ostr<<(short)guid.GetCoderUID()<<'-';
	ostr<<(short)guid.GetFrameUID()<<'-';
	ostr<<(short)guid.GetManipulatorUID();
	return ostr;
}

std::string toString(const char& obj,int base,int width, char fill)
{
	std::ostringstream ostr;
	ostr<<std::setbase(base)
		<<std::setw(width)<<std::setfill(fill)<<(short)obj;
	return ostr.str();
}

std::string toString(const byte& obj,int base,int width, char fill)
{
	std::ostringstream ostr;
	ostr<<std::setbase(base)<<std::setw(width)<<std::setfill(fill)<<(unsigned short)obj;
	return ostr.str();
}

