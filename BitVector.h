#ifndef BITVECTOR_H
#define BITVECTOR_H

#include <vector>

class CFile;

class CBitVector
{
public:
	/// Creates new instance of CBitVector class
	CBitVector();

	/// Length of the data in bytes (not including pending byte)
    int Length() const;

public:  //Write operations
	/// Appends rightmost aBits bits from aVal to the buffer
	void Append(unsigned int aVal, int aBits);
	
	/// Appends aBytes bytes to the buffer
	void Append(const void* aBuffer, int aBytes);

	/// Call this method when all data have been written to the buffer
	/** This method appends pending byte to the buffer if needed. */
	void End();

	/// Writes the buffer to the file at current position
	void AppendToFile(CFile& aFile);

	/// Read the buffer from the file. It does not reset the internal buffer
	void ReadFromFile(CFile& aFile,unsigned int aBytesCount);
	
public:  //Read operations
	/// Places current read position at the beginnig of the buffer
	void ResetRead();

	/// Gets aBits bits from the buffer
	unsigned int Get(int aBits);

private:
	/// Buffer
	std::vector<unsigned char> iBytes;

	/// Extracts leftmost aCount bits from aVal which has aTotalBits meaningfull bits
	unsigned int Left(unsigned int aVal, int aTotalBits, int aCount);

	// Write operations:

	/// Byte waiting for appending to the buffer
	unsigned char iPendingByte;

	/// Number of used bits in current byte
	int iBitsUsed;
	
	/// Number of not used bits in current byte
	int BitsLeft() const;

	/// Checks if pending byte is full, and if so, appends it to the buffer
	void CheckFullByte();
	
	/// Appends full byte to the buffer (assumes, that pending byte is empty)
	void AppendFullByte(unsigned char aVal);
	
	/// Appends bits to current byte, but not more that can fit in pending byte
	void AppendNotOverlapping(unsigned char aVal, int aBits);

	// Read opearations

	int iReadOffsetByte;
	int iReadOffsetBit;

	int ReadBitsLeft() const;
	void ReadCheckFullByte();
	unsigned int GetNotOverlapping(int aBits);
	unsigned int GetFullByte();

};

inline int CBitVector::Length() const
{
	return (int)iBytes.size();
}
inline int CBitVector::BitsLeft() const
{
	return 8-iBitsUsed;
}
inline unsigned int CBitVector::Left(unsigned int aVal, int aTotalBits, int aCount)
{
	return (aVal >> (aTotalBits-aCount)) & ((1<<aCount) - 1);
}
inline int CBitVector::ReadBitsLeft() const
{
	return 8-iReadOffsetBit;
}

#endif  //BITVECTOR_H
