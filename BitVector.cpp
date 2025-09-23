#include <assert.h>

#include "File.h"
#include "BitVector.h"


CBitVector::CBitVector()
{
	iPendingByte = 0;
	iBitsUsed = 0;

	ResetRead();
};

void CBitVector::Append(unsigned int aVal, int aBits)
{
	if( aBits <= BitsLeft() )
	{
		//trivial case
		AppendNotOverlapping( Left(aVal,aBits,aBits), aBits);
		return;
	}

	int leftend = BitsLeft();
	AppendNotOverlapping( Left(aVal, aBits, leftend), leftend );
	int fullbytes = (aBits - leftend) / 8;
	for(int i=0 ; i<fullbytes ; ++i)
	{
		AppendFullByte(Left(aVal, aBits-leftend- i*8, 8));
	}
	int rightend = aBits - leftend - 8*fullbytes;
	if( rightend > 0 )
	{
		AppendNotOverlapping( Left(aVal, rightend, rightend), rightend);
	}	
}

void CBitVector::Append(const void* aBuffer, int aBytes)
{
	//not a very efficient implementation
	const unsigned char* b = (const unsigned char*)aBuffer;
	for(int i=0;i<aBytes;++i)
		Append(b[i],8);
}

void CBitVector::End()
{
	if( iBitsUsed > 0 )
	{
		iBytes.push_back(iPendingByte);
		iBitsUsed = 0;
		iPendingByte = 0;
	}
}

void CBitVector::AppendToFile(CFile& aFile)
{
	assert(iBitsUsed == 0);

	for(unsigned int i=0;i<iBytes.size();++i)
	{
		unsigned char b = iBytes[i];
		aFile.Append(&b,1);
	}
}

void CBitVector::ReadFromFile(CFile& aFile,unsigned int  aBytesCount)
{
	for(unsigned int i=0;i<aBytesCount;++i)
	{
		unsigned char b;
		aFile.ReadNext(&b,1);
		AppendFullByte(b);
	}
}

void CBitVector::ResetRead()
{
	iReadOffsetByte = 0;
	iReadOffsetBit = 0;
}

unsigned int CBitVector::Get(int aBits)
{
	if( aBits <= 8-iReadOffsetBit )
	{
		//trivial case
		return GetNotOverlapping(aBits);
	}

	unsigned int val = 0;
	int leftend = ReadBitsLeft();
	val = GetNotOverlapping(leftend);
	int fullbytes = (aBits - leftend) / 8;
	for(int i=0;i<fullbytes;++i)
	{
		val = (val << 8) | GetNotOverlapping(8);
	}
	int rightend = aBits - leftend - 8*fullbytes;

	if( rightend > 0 )
	{
		val = (val << rightend) | GetNotOverlapping(rightend);
	}
	return val;
}


void CBitVector::CheckFullByte()
{
	if( iBitsUsed == 8 )
	{
		iBytes.push_back(iPendingByte);
		iPendingByte = 0;
		iBitsUsed = 0;
	}

}

void CBitVector::AppendFullByte(unsigned char aVal)
{
	assert(iBitsUsed==0);
	iBytes.push_back(aVal);
}

void CBitVector::AppendNotOverlapping(unsigned char aVal, int aBits)
{
	assert(aBits <= 8);
	assert(aBits <= 8-iBitsUsed);

	if( aBits > 0 )
	{
		int shift = BitsLeft() - aBits;
		iPendingByte |= (aVal << shift);
		iBitsUsed += aBits;

		CheckFullByte();
	}
}

void CBitVector::ReadCheckFullByte()
{
	if( iReadOffsetBit == 8 )
	{
		iReadOffsetBit = 0;
		++iReadOffsetByte;
	}
}

unsigned int CBitVector::GetNotOverlapping(int aBits)
{
	assert( ReadBitsLeft() >= aBits );

	int shift = ReadBitsLeft() - aBits;
	unsigned int val = ( iBytes[iReadOffsetByte] >> shift ) & (1<< (aBits-1) );
	iReadOffsetBit += aBits;

	ReadCheckFullByte();

	return val;
}

unsigned int CBitVector::GetFullByte()
{
	assert(iReadOffsetBit == 0);
	return iBytes[iReadOffsetByte++];
}

