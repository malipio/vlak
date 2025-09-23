// $Id: ientropycompressor.h,v 1.4 2005/11/15 15:43:49 wwasiak Exp $
#ifndef IENTROPYCOMPRESSOR_H
#define IENTROPYCOMPRESSOR_H
#include "icompressedframe.h"
#include "iframe.h"

// Interface IEntropyCompressor
// interfejs odpowiedzialny za kompresje ramek
class IEntropyCompressor 
{
public:
	virtual ~IEntropyCompressor() {};
	// skasowalem const'y - zeby mozna bylo operowac na argumencie
	virtual ICompressedFrame* CompressFrame(IFrame& frame) = 0;
		
	virtual IFrame* DecompressFrame(ICompressedFrame& frame) = 0;
};

class CSimpleCompressor : public IEntropyCompressor
{
public:
		
	virtual ICompressedFrame* CompressFrame(IFrame& frame)
	{
		// z lenistwa tak robimy, a takze zeby zachowac ogolnosc
		// taki proceder moze byc stosowany w miare bezbolesnie
		ICompressedFrame *tmp = dynamic_cast<ICompressedFrame *>(&frame);
		// mozna tez inaczej - poniewaz zakladamy ze nasz SimpleCompressor
		// kompresuje tylko CSimpleCompressedFramesy to mozna
		// utworzyc nowa ramke tego typu:
		// tmp = new CSimpleCompressedFrame(frame->GetInfo());
		// nie zawsze takie rozwiazanie jest mozliwe! 
		// (bo mozna stracic pewne istotne informacje z ramki 
		// ktore nie sa przechowywane w CBitStreamie zwracanym przez 
		// GetDataStream())

		// nie robimy zadnej kompresji
		tmp->SetCompressedDataStream(tmp->GetDataStream());
		return tmp;
	}
		
	virtual IFrame* DecompressFrame(ICompressedFrame& frame)
	{
		// TODO: konstruktor kopiujacy dla klas IFrame/ICompressedFrame !!!
		ICompressedFrame *tmp = &frame;
		tmp->SetDataStream( tmp->GetCompressedDataStream() );
		return tmp;
	}
};
#endif //IENTROPYCOMPRESSOR_H
