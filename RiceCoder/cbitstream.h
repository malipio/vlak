#ifndef BITSTREAM_H
#define BITSTREAM_H
#include <memory.h>
#include <ostream>

typedef unsigned char byte;

#define UNARY_TRESHOLD 8 // if we change it to more then 32 we should change the Read/Write functions

/// this class encapulates the bit stream functionality
class CBitStream
{
private:
	byte* streamStart;
	byte* streamEnd;
	int streamLength;

	/// Current pointer
	byte* ptr;
	/// bit offset in the current byte (0 .. 8)
	byte offset;

	/// This flag is set if we have to manage memory allocation/dellocation ourselves
	bool bManageAllocation;

	int BitsNeededToReprestent(unsigned int x);

public:
	CBitStream(void *stream, int length);

	/// If the stream is created that way, it will be automatically deleted on StreamDestruction
	CBitStream(int length);
	~CBitStream();
	
	inline byte * GetStreamStart() {return streamStart;};

	// jesli fullCopy = false to robimy kopie od aktualnej pozycji w strumieniu
	CBitStream(const CBitStream& stream, bool fullCopy = true)
	{
		if(fullCopy) // tradycyjny konstruktor kopiujacy
		{
			streamLength = stream.streamLength;
			streamStart = new byte[streamLength];
			// kopiuj pamiec
			memcpy(streamStart,stream.streamStart,streamLength);
		} else {
			// robimy kopiowanie, ale nie wskaznikow :P
			// kopiujemy strumien od miejsca gdzie jest aktualny wskaznik!!!
			streamLength = stream.streamEnd-stream.ptr;
			streamStart = new byte[streamLength];			
			// kopiuj pamiec
			memcpy(streamStart,stream.ptr,streamLength);
		}
		
		// copy&paste
		streamEnd = streamStart + streamLength;
		ptr = streamStart;
		offset = 0; // bitami sie nie zajmujemy
		bManageAllocation = true;

	}

	CBitStream &operator=(CBitStream& stream)
	{
		// delete previous , ignore manage allocation
		if(streamStart != NULL)
			delete [] streamStart;

		streamLength = stream.streamLength;
		streamStart = new byte[streamLength];
		memcpy(streamStart,stream.streamStart,streamLength);
		streamEnd = streamStart + streamLength;
		ptr = streamStart;
		offset = 0;
		bManageAllocation = true;
		return *this;
	}

	// TODO: operator =
	/// Read N bits from the stream (and proceeds next)
	int ReadNBits(void *output, int count, bool bPeak = false); 

	int WriteNBits(void *input, int count);

	/// this method is tricky it writes the first count bits from the number in the correct order (Big/Little Endian problem)
	int WriteUIntNBits(unsigned int number, int count);

	int ReadUIntNBits(unsigned int *output, int count);

	/// Writes unary number
	int WriteUnaryNumber(unsigned int number);

	unsigned int ReadUnaryNumber();

	bool EndOfStream(){return (ptr>=streamEnd);};
	
	/// returns current position of the buffer in bytes 
	/// TODO: return bits
	int CurrentPosition(){return (int)(ptr-streamStart);};

	/// Moves the Read/Write ptr by nCount bits
	int MoveNBits(int nCount);

	/// Moves the Read/Write ptr by nCount bytes
	int Move(int nCount);

	byte* GetBytes() const;

	int BitSize();

	/// Returns the size in bytes of the stream
	int Size() const;

	/// Writes one byte to the end of stream (where ptr points)
	int Write(byte input);

	/// Writes nCount bytes to the end of stream (where ptr points)
	int Write(void *input, int nCount);

	/// Gets the byte, and moves the current ptr
	int GetNextByte(byte *output);

	/// Gets the nCount bytes, and move the current ptr
	int GetNextNBytes(byte *output, int nCount);

	/// Returns the ptr to the specified location 
	/// Is it really necessary??
	byte* GetBytesAtPos(int index);

	/// Moves the ptr position at the begining
	void Reset();

	/// Prints data...
	std::ostream& Print(std::ostream& os) const;
};

#endif
