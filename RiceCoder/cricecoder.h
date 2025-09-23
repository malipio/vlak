#ifndef CRiceCoder_H
#define CRiceCoder_H

#include "../cbitstream.h"
#include "../ientropycompressor.h"
#include <queue>
using namespace std;
#define PREV_SAMPLES_LENGTH 16

	
/// This is the class that encapsulates Rice Coder functionality
/**
	For now it only support k parameter up to 8 bits.
	It only supports signed integers 
	TODO: Describe Rice stream format here
*/
class CRiceCoder: public IEntropyCompressor
{
public:
	enum NumberFormat
	{
		E8Bits,
		E16Bits,
		E32Bits
	};

private:
	NumberFormat m_format;
	int inLength, outLength;
	queue<unsigned int> m_prevSamples;
	int m_prevSamplesBitCount;

	/// k parameter used to determine how many LSBits to cut
	int kParam;

	/// This function encodes an input number and stores it in the output
	int EncodeNumber(void* input, CBitStream* output);

	int DecodeNumber(CBitStream* input, void* output);

	/* Get/Set operations */
	void SetNumberFormat(NumberFormat format){this->m_format = format;};

	int DetermineAndSetInitialKParam(CBitStream &input, int sampleSize);

	int GetBitCount(unsigned int number);

public:
	CRiceCoder(void);
	virtual ~CRiceCoder();

	static unsigned long CheckProcessMemInfo();
	/// Compresses the in buffer using Rice coder
	/*int Compress(void* in, int inLen, void* out, int outLen);

	/// Decompresses the in buffer using Rice coder
	int Decompress(void* in, int inLen, void* out, int outLen);*/

	
	virtual ICompressedFrame* CompressFrame(IFrame& frame);
		
	virtual IFrame* DecompressFrame(ICompressedFrame& frame);
};
#endif
