// $Id: BlocksProviderFromWaveReader.h,v 1.4 2005/11/15 15:10:54 wwasiak Exp $
#ifndef BLOCKSPROVIDERFROMWAVEREADER_H
#define BLOCKSPROVIDERFROMWAVEREADER_H

#include "cblock.h"
#include "iblocksprovider.h"

class CWaveReader; //potrzebna taka zapowiedz - gupi hack - ale nie mozna zainkludowac wavefile.h

// dziedziczy po interfejsie IBlocksProvider
// interfejs odpowiedzialny za dostarczanie kolejnych blokow z pliku
class BlocksProviderFromWaveReader : public IBlocksProvider
{
private:
	CWaveReader * pMyWaveReader;
public:
	BlocksProviderFromWaveReader(CWaveReader * pMyNewWaveReader); //nie ma konstruktora bezargumentowego
	virtual CBlock GetNextBlock();
	virtual bool HasNext() const;
	unsigned int iBlockSize;
};
#endif //BLOCKSPROVIDERFROMWAVEREADER_H
