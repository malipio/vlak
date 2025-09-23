#include "RiceCoderTests.h"
#include "cricecoder.h"
#include "../csimplecompressedframe.h"
#include <iostream>
using namespace std;

#define TEST_LENGTH 1024

CRiceCoderTests::CRiceCoderTests(void)
{
}

CRiceCoderTests::~CRiceCoderTests(void)
{
}

bool CRiceCoderTests::TestAll()
{
	bool bPassed = true;
	cout << "Test klasy CRiceCoder..." << endl;
	bool b16BitPassed = Test16Bit();	
	
	if(b16BitPassed == false) // || b16bit etc
		bPassed = false;
	
	return bPassed;
}

bool CRiceCoderTests::Test16Bit()
{
	cout << "Test 16 Bitowych wartosci" << endl;
	bool bPassed = true;
	
	CBlock block(TEST_LENGTH, 16, 44100, 1);	
	for(int i=0; i<TEST_LENGTH; i++)
	{
		block.SetSample(0, i, i*8);
	}

	CSimpleCompressedFrame frame(block);
	
	CRiceCoder coder;
	ICompressedFrame *comprFrame = coder.CompressFrame(frame);
	IFrame* outFrame = coder.DecompressFrame(*comprFrame);
	
	CBitStream outStream = frame.GetDataStream();
	if(outStream.Size() != TEST_LENGTH * 2)
	{
		cout << "\tError: Input/Output stream sizes don't match. Input length: " << TEST_LENGTH << " Output length: " << outStream.Size();
		delete comprFrame;
		return false;
	}

	CBitStream inputStream = frame.GetDataStream();
	byte* inbuf = inputStream.GetBytes();
	byte* outbuf = outStream.GetBytes();

	for(int i = 0; i<TEST_LENGTH; i++)
	{
		if(inbuf[i] != outbuf[i])
		{
			bPassed = false;
			cout << "\tError: Input and Output items don't match";
			break;
		}
	}

	return bPassed;
}

bool CRiceCoderTests::Test8Bit()
{
	return true;
}

bool CRiceCoderTests::Test32Bit()
{
	return true;
}