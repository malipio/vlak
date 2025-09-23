// $Id: cwavfile.h,v 1.5 2005/11/15 15:43:49 wwasiak Exp $
#ifndef CWAVFILE_H
#define CWAVFILE_H
#include "cblock.h"
#include "iinfo.h"
#include "iblocksprovider.h"

// Class CWavFile
// doszlismy do wniosku ze ta klasa jest nieprzydatna :(
// dlatego zrobilem podzial na 2 interfejsy, z ktorych obecnie
// nalezy korzystac
class CWavFile 
{
public:
	void WriteBlock(const CBlock& block);
	void WriteHeader(const IInfo& header);
	
	// TODO: metoda ta wystarczy ze za kazdym razem bedzie
	// zwracala ten sam obiekt, a wiec np. atrybut prywatny naszej klasy
	// blockSize - rozmiar bloku *w probkach* na kanal
	IBlocksProvider* GetBlocksProvider (int blockSize); // const ??

	// TBD: do pliku bedzie mozna albo tylko pisac albo tylko czytac
	// co znacznie upraszcza stworzenie tej klasy
};

// a tu mamy interfejsy - na prosbe wojtka
class IWavFileReader
{
public:	
	virtual ~IWavFileReader() {};
	// wszystkie informacje bedzie nam zwracal BlocksProvider
	// chodzi tu takze o informacje dotyczace probek
	virtual IBlocksProvider* GetBlocksProvider (int blockSize) = 0;
};

class IWavFileWriter
{
public:
	virtual ~IWavFileWriter() {};
	virtual void WriteBlock(const CBlock& block) = 0;
	virtual void WriteHeader(const IInfo& header) = 0;
};

#endif //CWAVFILE_H

