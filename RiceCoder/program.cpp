// program.cpp : Defines the entry point for the console application.
//#include "stdafx.h"
#include "RiceCoderTests.h"
#include "cbitstream.h"

int main(int argc, char* argv[])
{
	CBitStream stream(1024);


	unsigned int x;
	byte *p = stream.GetBytes();
	for(unsigned int i = 0; i<1024; i++)
	{
		p[8] = i % 255;
	}

	CBitStream second(0);
	second = stream;
	int cmp = memcmp(stream.GetBytes(), second.GetBytes(), stream.Size());

	int length = stream.CurrentPosition();
	CRiceCoderTests test;
	test.TestAll();

	/*coder.SetNumberFormat(CRiceCoder::E16Bits);
	int size = coder.Compress((void*)buffer.GetBytes(), buffer.Size(), (void*)WriteBuffer.GetBytes(), WriteBuffer.Size());
	CBitStream buf2(buffer.Size());
	coder.Decompress((void*)WriteBuffer.GetBytes(), size, (void*)buf2.GetBytes(), buf2.Size());
	buffer.Reset();

	for(int i = 0; i<64; i++)
	{
		short num1;
		short num2;
		buffer.GetNextNBytes((byte*)&num1, 2);
		buf2.GetNextNBytes((byte*)&num2, 2);
		if(num1 != num2)
			int a = 0;
	}*/

	// TODO: Write tests

	return 0;
}
