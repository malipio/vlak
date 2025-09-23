// $Id: icompressedframe.h,v 1.10 2005/11/27 15:06:44 pmalinow Exp $
#ifndef ICOMPRESSEDFRAME_H
#define ICOMPRESSEDFRAME_H
#include "iinfo.h"
#include "iframe.h"
#include "cbitstream.h"
#include "uid.h"

// Interface ICompressedFrame
// interfejs odpowiadajacy skompresowanej ramce
class ICompressedFrame : public IFrame 
{
public:
	virtual ~ICompressedFrame() {};
	// ustawia skompresowany strumien
	virtual void SetCompressedDataStream (const CBitStream& compressedDataStream) = 0;
	
	// pobiera skompresowany strumien w celu dekompresji
	virtual const CBitStream& GetCompressedDataStream() const = 0;
	
	// serializuje obiekt do strumienia
	virtual CBitStream Serialize() const = 0;
	// a moze tak lepiej?
	//virtual void Serialize(CBitStream *bitStream) const = 0;
	
	// tworzy nowy obiekt klasy IFrame (i pochodnych) na podstawie strumienia
	// danych. UWAGA: chodzi o strumien zwracany przez metode Serialize()
	static ICompressedFrame* Deserialize (CBitStream& stream, const IInfo& info);
	
protected:
	static ICompressedFrame* CreateInstance(const GUID guid,const IInfo& info, CBitStream& stream);
};

#endif //ICOMPRESSEDFRAME_H

