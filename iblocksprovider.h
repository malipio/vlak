// $Id: iblocksprovider.h,v 1.4 2005/11/15 15:43:49 wwasiak Exp $
#ifndef IBLOCKSPROVIDER_H
#define IBLOCKSPROVIDER_H
#include "cblock.h"

// Interface IBlocksProvider
// interfejs odpowiedzialny za dostarczanie kolejnych blokow z pliku
// jak to zostanie zrobione to juz problem programisty a nie projektanta <lol>
class IBlocksProvider {
public:
	// TODO: lepiej zwracac wskaznik albo referencje? 
	// (jaka szkoda ze to nie java...)
	virtual CBlock GetNextBlock() = 0;

	// musimy miec warunek koncowy iterowania, prawda? :)
	virtual bool HasNext() const = 0;
	
	virtual ~IBlocksProvider() {};
};
#endif //IBLOCKSPROVIDER_H

