// RiceCoder.cpp 
#include "cricecoder.h"
#include "../csimplecompressedframe.h"
#include <memory.h>
#include <fstream>


struct RICE_HEADER
{
	byte k;
	//int size; //in bits
	int decompressedSize; // in bytes
};

CRiceCoder::CRiceCoder(void)
{
	kParam = 8;
	m_format = E32Bits;
	m_prevSamplesBitCount = 0;
}

CRiceCoder::~CRiceCoder(void)
{
}

/**
	\param in Input buffer
	\param inLen Length, in bytes, of the input buffer	
	\param out the buffer that will receive compressed stream
	\param outLen shouldn't be smaller than inLen + 1
*/
/*int CRiceCoder::Compress(void* in, int inLen, void* out, int outLen)
{
	// TODO: Write k param to the 
	CBitStream output(out, outLen);

	byte* p = (byte*) in;
	byte* end = p + inLen;
	while(p < end)
	{
		EncodeNumber(p, &output);

		switch(this->m_format)
		{
		case E8Bits:
			p++;
			break;
		case E16Bits:
			p+=2;
			break;
		}
		// TODO: check if the p + 2/4 is ok
	}
	

	return output.CurrentPosition();
}

int CRiceCoder::Decompress(void* in, int inLen, void* out, int outLen)
{
	CBitStream input(in, inLen);
	byte* p = (byte*) out;	
	byte* end = p + outLen;

	while(!input.EndOfStream())
	{
		switch(this->m_format)
		{
		case E8Bits:
			if(p >= end) return -1;
			break;
		case E16Bits:
			if(p+1 >= end) return -1;
			break;
		}

		int num = DecodeNumber(&input, p);
		
		switch(this->m_format)
		{
		case E8Bits:
			p++;
			break;
		case E16Bits:
			p+=2;
			break;
		}

	}
	

	return 0;
}*/

/**
	It assumes that the buffers are accessible. It only checks if k is grater 
	then the number of bits.
	We must encode change the value of signed integers, because -1 is 0xFFFFFFF and can't
	be efficiently compressed. So all the positive numbers are 2,4,6 etc, and negative are 1,3,5 etc
*/
int CRiceCoder::EncodeNumber(void* input, CBitStream* output)
{
	// TODO: check the k parameter
	// TODO: Create RiceHeader
	unsigned int num = 0;
	int x;
	switch(this->m_format)
	{
	case E8Bits:
		x = (int)*((char*)input);
		break;
	case E16Bits:
		x = (int)*((short*)input);
		break;
	case E32Bits:
		x = (int)*((int*)input);
		break;
	}

	num = (unsigned int)(x < 0? ((-x)<<1)-1: x<<1);
	
	unsigned int mask = (0x1 << kParam) - 1; // create a mask to cut
	unsigned int kLSBits = (num & mask) << (32 - kParam); // get the LSBits
	unsigned int rest = num >> kParam; // cut the k LSB

	// Unary code the number
	if( output->WriteUnaryNumber(rest) == -1)
		return -1;

	if( output->WriteUIntNBits(kLSBits, kParam) == -1)
		return -1;

	// update kParam
	int nBitCount = GetBitCount(num);
	m_prevSamples.push(nBitCount);
	m_prevSamplesBitCount += nBitCount;
	
	
	if(m_prevSamples.size() > PREV_SAMPLES_LENGTH)
	{
		m_prevSamplesBitCount -= m_prevSamples.front();
		m_prevSamples.pop();

		kParam = m_prevSamplesBitCount / PREV_SAMPLES_LENGTH;

	} 
	return 0;
}

int CRiceCoder::DecodeNumber(CBitStream* input, void* output)
{
	// TODO: Read rice header (sotre information about the bits)

	unsigned int rest = input->ReadUnaryNumber();
	unsigned int kLSBits = 0; // todo read kLSBits
	input->ReadUIntNBits(&kLSBits, kParam);
	
	unsigned int num = (rest << kParam) + (kLSBits >> (32 - kParam));

	// convert to int
	switch(this->m_format)
	{
	case E8Bits:
		//x = (int)*((char*)num);
		((char*)output)[0] = (char)((num & 1)? -(int)((num+1)>>1): (num>>1)); // TODO: check compilation warnings
		break;
	case E16Bits:
		//x = (int)*((short*)input);
		((short*)output)[0] = (short)((num & 1)? -(int)((num+1)>>1): (num>>1));
		break;
	case E32Bits:
		((int*)output)[0] = (int)((num &1)? -(int)((num+1)>>1): (num>>1));
		break;
	}

	//update kParam
	int nBitCount = GetBitCount(num);
	m_prevSamplesBitCount += nBitCount;
	m_prevSamples.push(nBitCount);

	if(m_prevSamples.size() > PREV_SAMPLES_LENGTH)
	{
		m_prevSamplesBitCount -= m_prevSamples.front();
		m_prevSamples.pop();
		
		kParam = m_prevSamplesBitCount / PREV_SAMPLES_LENGTH;
	}
	return 0;
}

//HANDLE GetCurrProcessHandle()
//{
//	int PID = 0;
//	static HANDLE hProcess = 0;
//	if( hProcess == 0)
//		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |PROCESS_VM_READ,FALSE, PID);
//	return hProcess;
//}

unsigned long CRiceCoder::CheckProcessMemInfo()
{
	return 0;
	/*PROCESS_MEMORY_COUNTERS pmc;
	HANDLE hProcess = GetCurrProcessHandle();
	BOOL bRet = GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));
	return pmc.WorkingSetSize;*/
}

ICompressedFrame* CRiceCoder::CompressFrame(IFrame& frame)
{
	//clear the queue
	int qSize = m_prevSamples.size();
	for(int i = 0; i < qSize; i++)
	{
		m_prevSamples.pop();
	}
	m_prevSamplesBitCount = 0;

	CBitStream input = frame.GetDataStream();
	CBitStream *output = new CBitStream(input.Size() + sizeof(RICE_HEADER)); // TODO: Determine how much we must add 
	
	//CBitStream output(out, outLen);
	byte* p = (byte*) input.GetBytes();
	byte* end = p + input.Size();

	kParam = DetermineAndSetInitialKParam(input, sizeof(CExtSample));
	RICE_HEADER header;
	header.k = kParam;
	header.decompressedSize = input.Size();

	bool bExceeded = false;

	while(p < end)
	{
		/** TODO: Remove TEST ***/
		/*int sample, outSample;
		sample = (int)*(int*)p;

		int currPtr = output->BitSize();
		byte prevKParam = kParam;*/
		
		if( EncodeNumber(p, output) == -1 )
		{
			bExceeded = true;
			break;
		}
		/*byte currKParam = kParam;
		kParam = prevKParam;
		
		output->MoveNBits(-(output->BitSize() - currPtr));
		DecodeNumber(output, &outSample);
		if(sample != outSample)
		{
			int a = 0;
		}

		kParam = currKParam;*/
		

		switch(this->m_format)
		{
		case E8Bits:
			p++;
			break;
		case E16Bits:
			p+=2;
			break;
		case E32Bits:
			p+=4;
		}
		// TODO: check if the p + 2/4 is ok
	}

	if(bExceeded)
	{
		// copy without compress
		header.k = 0;
		//output->Reset();
		//output->Move(sizeof(RICE_HEADER));
		//output->Write(input.GetBytes(), input.Size());
	}
	
	int streamSize = ((output->BitSize() - 1) / 8) + 1;
	CBitStream out(sizeof(RICE_HEADER) + streamSize);
	out.Write((void*)&header, sizeof(RICE_HEADER));
	if(bExceeded)
		out.Write((void*)input.GetBytes(), input.Size());
	else
		out.Write((void*)output->GetBytes(), streamSize);

	ICompressedFrame *outputFrame = dynamic_cast<ICompressedFrame *>(&frame);
	//CSimpleCompressedFrame *outputFrame = new CSimpleCompressedFrame(frame.GetInfo());
	out.Reset(); 
	outputFrame->SetCompressedDataStream(out);
	delete output; // FOr now delete

	return outputFrame;
}

/**
	Always set the input pointer to the begining
	*/
int CRiceCoder::DetermineAndSetInitialKParam(CBitStream &input, int sampleSize)
{
	//First method - Get first 16 samples
	int x = 0;
	unsigned int sample = 0;
	int nBitCount = 0;
	bool bExceeded = false;
	int nSamplesRead;
	for(nSamplesRead = 0; nSamplesRead < PREV_SAMPLES_LENGTH; nSamplesRead++)
	{
		if( input.ReadNBits(&x, sampleSize*8) == -1)
		{
			bExceeded = true;
			break;
		}

		sample = (unsigned int)(x < 0? ((-x)<<1)-1: x<<1);
		nBitCount += GetBitCount(sample);
	}
	
	if(bExceeded)
		kParam = 0;
	else
	{
		kParam = (nBitCount + 1) / nSamplesRead;
		if(kParam == 0)
			kParam = 1;
	}
	
	input.Reset();

	return kParam;
}

int CRiceCoder::GetBitCount(unsigned int number)
{
	int ret=0;
	for(ret=32; ret>0 && !(number & 0x80000000); ret--)
		number <<= 1;
	return ret;
}

IFrame* CRiceCoder::DecompressFrame(ICompressedFrame& frame)
{
	int qSize = m_prevSamples.size();
	for(int i = 0; i < qSize; i++)
	{
		m_prevSamples.pop();
	}
	m_prevSamplesBitCount = 0;

	CBitStream input = frame.GetCompressedDataStream();
		
	// TODO: Get output size
	RICE_HEADER header;
	input.GetNextNBytes((byte*)&header, sizeof(RICE_HEADER));
	kParam = header.k;
	CBitStream *output = new CBitStream(header.decompressedSize); 
	int nSamples = 0;
	switch(this->m_format)
	{
	case E16Bits:
		nSamples = header.decompressedSize/2;
		break;
	case E32Bits:
		nSamples = header.decompressedSize/4;
		break;
	}

	if(kParam == 0) // no compresion made
	{
		byte *ptr = input.GetBytesAtPos(input.CurrentPosition());
		output->Write(ptr, header.decompressedSize);
	}
	else
	{
		// TODO: Read rice header (sotre information about the bits)
		// what happens on the end of the stream!
		byte* p = output->GetBytes();	
		byte* end = p + output->Size();

		while(nSamples != 0 && !input.EndOfStream())
		{
			switch(this->m_format)
			{
			case E16Bits:
				if(p+1 >= end) return 0;
				break;
			case E32Bits:
				if(p+3 >= end) 
					break; // TODO: better input consumption
				break;
			}

			if( DecodeNumber(&input, p) == -1)
			{
				break; //TODO: Error handling
			}
			nSamples--;

			switch(this->m_format)
			{
			case E16Bits:
				p+=2;
				break;
			case E32Bits:
				p+=4;
				break;
			}
		}
	}

	// TODO: konstruktor kopiujacy dla klas IFrame/ICompressedFrame !!!
	// nie ma takowego i nie bedzie (przyp. pmalinow)
	ICompressedFrame *tmp = &frame;
	tmp->SetDataStream( *output );
	delete output;

	return tmp;
}
