#include "cbitstream.h"
#include <memory.h>
#include <math.h>
#include <iostream>
#include "../streamops.h"

CBitStream::CBitStream(void *stream, int length)
{
	streamStart = (byte*)stream;
	this->streamLength = length;
	streamEnd = streamStart + length;
	ptr = streamStart;
	offset = 0;
	bManageAllocation = false;
}

CBitStream::CBitStream(int length)
{
	streamStart = new byte[length];
	this->streamLength = length;
	streamEnd = streamStart + length; 
	// czyli streamEnd to wskaznik na pierwszy bajt poza tablica!
	ptr = streamStart;
	offset = 0;
	bManageAllocation = true;
}

CBitStream::~CBitStream(void)
{
	if(bManageAllocation)
	{
		delete [] streamStart;
		streamStart = NULL;
	}
}

int CBitStream::ReadNBits(void *output, int count, bool peak /* = false */)
{
	if(ptr + ((count + offset)/8) > streamEnd)
	{
		return -1;
	}
	if( (ptr + ((count + offset)/8) == streamEnd) )
	{
		if((offset + count) % 8 != 0)
			return -1;
	}

	byte* oldPtr = ptr;
	byte* outPtr = (byte*)output;
	byte revOffset = 8 - offset;
	byte mask = (0x1 << (revOffset)) - 1;
	while(count > 7) //copy whole bytes
	{
		if(revOffset == 8)
			*outPtr = ((ptr[0] & mask) << offset);
		else
			*outPtr = ((ptr[0] & mask) << offset)+(ptr[1] >> revOffset);
		//*outPtr = outByte;
		outPtr++;
		count -= 8;
		ptr++;
	}
	// copy the rest and update offset ptr;
	byte oldOffset = offset;

	if( offset + count > 7)
	{
		offset = (offset + count) % 8;
		byte newMask = (0xFF<<(8-offset));
		if(offset != 0)
			*outPtr = ((ptr[0] & mask)<< oldOffset)+((ptr[1]&newMask)>>revOffset); // (8 - offset) is a new revOffset
		else
			*outPtr = ((ptr[0] & mask)<< oldOffset);
		ptr++;
	}
	else if(count != 0)
	{
		offset += count;
		*outPtr = ((ptr[0] & mask)>>(8-offset))<< (8-offset + oldOffset); 
	}

	if(peak)
	{
		ptr = oldPtr;
		offset = oldOffset;
	}

	return 0;
}

int CBitStream::WriteNBits(void *input, int count)
{
	if(ptr + ((count + offset)/8) > streamEnd)
	{
		return -1;
	}
	if( (ptr + ((count + offset)/8) == streamEnd) )
	{
		if((offset + count) % 8 != 0)
			return -1;
	}

	byte* inPtr = (byte*)input;
	byte revOffset = 8 - offset;
	byte mask = (0xFFFFFFFF)<<offset;
	byte clearMask = ~(mask>>offset);//mask used to clear old values
	while(count > 7) //copy whole bytes
	{
		
		// zero the part that we want to write
		ptr[0] &= clearMask;
		ptr[0] += ((inPtr[0] & mask) >> offset);
		if(revOffset != 8)
			ptr[1] = (inPtr[0] << revOffset); // TODO: protect the end
		//*outPtr = outByte;
		inPtr++;
		count -= 8;
		ptr++;
	}
	// copy the rest and update offset ptr;
	if( offset + count > 7)
	{
		ptr[0] &= clearMask;
		ptr[0] += ((inPtr[0] & mask) >> offset);
		if(revOffset != 8 && count != 1)
			ptr[1] = (inPtr[0] << (revOffset));
		offset = (offset + count) % 8;
		//*outPtr = ((ptr[0] & mask)<< revOffset)^(ptr[1]>>(8 - offset)); // (8 - offset) is a new revOffset
		ptr++;
	}
	else if(count != 0)
	{
		byte newMask = (0xFFFFFFFF) << (8 - count); // copyt the last bytes
		ptr[0] &= clearMask;
		ptr[0] |= ((inPtr[0] & newMask) >> offset);
		offset += count;
	}

	return 0;
}

int CBitStream::WriteUIntNBits(unsigned int number, int count)
{
	// change byte order of the unsigned int
	byte* p = (byte*)&number;
	unsigned int num = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
	return WriteNBits(&num, count);
}

int CBitStream::ReadUIntNBits(unsigned int *number, int count)
{
	unsigned int num = 0;
	if( ReadNBits(&num, count) == -1)
		return -1;
	// change byte order of the unsigned int
	byte* p = (byte*)&num;
	*number = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
	return 0;
}

int CBitStream::WriteUnaryNumber(unsigned int number)
{
	unsigned int x = number;
	if(x > UNARY_TRESHOLD)
	{
		/// Read Manual PDF
		unsigned int one = 0xFFFFFFFF;
		unsigned int zero = 0;

		if( WriteNBits(&one, UNARY_TRESHOLD) == -1)
			return -1;

		x -= UNARY_TRESHOLD;

		int nLSBits = BitsNeededToReprestent(x);
		if( WriteNBits(&one, nLSBits) )
			return -1;
		if( WriteNBits(&zero, 1) )
			return -1;

		//x = number;
		x <<= (32 - nLSBits);
		//WriteNBits(&x, nLSBits);*/
		for(int i = 0; i < nLSBits; i++)
		{
			byte bitValue = (x & 0x80000000)?0x80:0;// This is unefficient, but it is rare situation, so...
			if( WriteNBits(&bitValue, 1) == -1)
				return -1;
			x <<= 1;
		}		
	}
	else
	{

		byte* buf = new byte[(number/8 + 1)];
		byte* p = buf;
		unsigned int x = number;

		while(x > 7)
		{
			*p++ = 0xFF;
			x -= 8;
		}

		*p = 0xFF << (8 - x);

		if( this->WriteNBits(buf, number + 1) )
			return -1;
		delete []buf;
	}
	return 0;
}

int CBitStream::BitsNeededToReprestent(unsigned int x)
{
	int ret=0;
	for(ret=32; ret>0, !(x & 0x80000000); ret--)
		x <<= 1;
	return ret;
}

unsigned int CBitStream::ReadUnaryNumber()
{
	unsigned int num = 0;
	byte tmp = 0;
	int tresholdBits = UNARY_TRESHOLD + 1;
	do
	{
		if( ReadNBits(&tmp, 8, true) == -1)
		{
			// we're at the end, so we cannot read whole byte
			ReadNBits(&tmp, 8 - offset, true);
		}
		if(tmp != 0xFF)
		{
			break;
		}
		else
		{
			MoveNBits(8);
			num += 8;
		}
	}while(1);
	
	// read the last byte
	byte mask = 0x80;
	while(tmp & mask)
	{
		mask = mask >> 1;
		num++;
	}

	//TODO: Create efficient way to move te seek ptr in the stream
	MoveNBits(num%8 + 1);


	if(num > UNARY_TRESHOLD)
	{
		byte bitValue;
		int nLSBits = num - UNARY_TRESHOLD;
	
		num = 0;
		for(int i=nLSBits; i>0;i--)
		{
			if( ReadNBits(&bitValue, 1) == -1)
				return -1;

			if(bitValue == 0)
				num = (num << 1 );
			else
				num = (num << 1 ) | 1;
		}
		num += UNARY_TRESHOLD;
	}
	return num;
}

/**
	Move the read/write ptr foward/backward (if using negative)
	\return 0 if OK, -1 if to end of stream found
*/
int CBitStream::MoveNBits(int nCount)
{
	int moveBytes = nCount/8;
	int moveBits = nCount % 8;
	ptr += moveBytes;
	offset += moveBits;
	if(offset > 7)
	{
		nCount<0?ptr--:ptr++;

		offset %= 8;
	}
	if(ptr < streamStart)
	{
		ptr = streamStart;
		return -1;
	} 
	if(ptr>streamEnd || (ptr == streamEnd && offset != 0)) // TODO: check if second condition is valid 
	{
		ptr = streamEnd;
		return -1;
	}

	return 0;
}


	
int CBitStream::Move(int nCount)
{
	if(offset == 0)
	{
		ptr += nCount;
	}
	else
	{
		MoveNBits(nCount * 8);
	}

	if(ptr > streamEnd)
	{
		ptr = streamEnd;
		return -1;
	}

	if(ptr < streamStart)
	{
		ptr = streamEnd;
		return -1;
	}

	return 0;
}

byte* CBitStream::GetBytes() const
{
	return this->streamStart;
}

int CBitStream::BitSize()
{
	int bits = (ptr-streamStart)*8;
	bits+=offset;
	return bits;
}

/// Returns the size in bytes of the stream
int CBitStream::Size() const
{
	return this->streamLength;
}

int CBitStream::Write(byte input)
{
	if(ptr >= streamEnd)
		return -1;

	int ret = 0;
	if(offset == 0)
	{
		*ptr = input;
		ptr++;
	}
	else
	{
		ret = WriteNBits(&input, 8);
	}

	return ret;
}

int CBitStream::Write(void *input, int nCount)
{
	if( ptr + nCount > streamEnd)
		return -1;

	int ret = 0;
	if(offset == 0)
	{
		memcpy((void*)ptr, input, nCount);
		ptr += nCount;
	}
	else
	{
		ret = WriteNBits(&input, nCount * 8);
	}

	return ret;
}

/// Gets the byte, and moves the current ptr
int CBitStream::GetNextByte(byte *output)
{
	if(ptr >= streamEnd)
		return -1;

	if(offset == 0)
	{
		*output = *ptr;
		ptr++;
	}
	else
	{
		if( ReadNBits(output, 8) != 0)
			return -1;
	}

	return 0;
}

/// Gets the nCount bytes, and move the current ptr
int CBitStream::GetNextNBytes(byte* output, int nCount)
{
	if(ptr + nCount > streamEnd)
		return -1;

	if(offset == 0)
	{
		memcpy(output, ptr, nCount);
		ptr += nCount;
	}
	else
	{
		if( ReadNBits(output, nCount * 8) != 0)
			return -1;
	}

	return 0;
}

byte* CBitStream::GetBytesAtPos(int index)
{
	if(index > streamLength)
		return 0;

	return streamStart + index;
}

void CBitStream::Reset()
{
	ptr = streamStart;
	offset = 0;
}

std::ostream& CBitStream::Print(std::ostream& os) const
{
	os << "CBitStream {" <<std::endl;
	os << "size= " << Size() << std::endl;
	os << "pos=" << ((int)(ptr-streamStart)) << std::endl; // TODO: czemu nie dziala?
	os << "stream {" << std::endl << std::endl;
	PrintArray(os,streamStart,streamLength,16,2," ",25);
	os << "}"<<std::endl;
	os << "};"<<std::endl;
	return os;
}

