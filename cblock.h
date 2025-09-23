// $Id: cblock.h,v 1.9 2005/11/26 19:10:58 pmalinow Exp $
#ifndef CBLOCK_H
#define CBLOCK_H
#include "iinfo.h"
#include "cblockinfo.h"
#include "csample.h"
#include "uid.h"

// Class CBlock
// blok sklada sie z wielu probek dla wielu kanalow.
// TODO: uzupelnic o konstruktory i skladowe prywatne
class CBlock
{
public:
	//ustanawia rozmiar bloka - bedzie on staly; nie ma konstruktora domyslnego
	CBlock (int Size, char bits=16 ,int freq=44100 ,char chan=2);
	CBlock(const IInfo& info,int blockSize);
	CBlock(const CBlock&);
	~CBlock();

	inline void SetManipulatorUID(char mUID) {manipUID = mUID;}
	inline char GetManipulatorUID() const { return manipUID; }
	CBlock operator=(const CBlock&);
	
	CSample* GetSamples(int chNum) const;

	// ta metoda jest jednak potrzebna:
	void SetSample(int chNum,int sampleNum, CSample sampleValue)
	{
		Data[BlockSize*chNum+sampleNum] = sampleValue;
	}
	// UWAGA: jesli w sygnaturze metody jest cos takiego: metoda(const CBlock& block)
	// to oznacza ze SetSample nie moze byc wywolane na takim obiekcie - i bardzo dobrze!

	const IInfo& GetInfo() const;

	// zwraca rozmiar bloku *w probkach* na kanal
	int GetBlockSize() const;
private:
	int BlockSize;
	CBlockInfo MyInfo;
	char manipUID;
	CSample * Data; //tablica z danymi - kanaly leza w niej jeden po drugim a nie naprzemiennie po samplu
};
#endif //CBLOCK_H
