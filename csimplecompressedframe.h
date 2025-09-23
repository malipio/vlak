// $Id: csimplecompressedframe.h,v 1.6 2005/11/26 19:10:58 pmalinow Exp $
#ifndef CSIMPLECOMPRESSEDFRAME_H
#define CSIMPLECOMPRESSEDFRAME_H
#include "uid.h"
#include "icompressedframe.h"
#include "csample.h"
#include "cblock.h"
#include "streamops.h"
#include <iostream>
using std::endl;

// Przykladowa implementacja
// UID = 0
class CSimpleCompressedFrame : public ICompressedFrame
{
private:
	CSample *samples;
	const IInfo& info;
	int blockSize;
	CBitStream *compressed;
	GUID guid;

	inline int _kw(int k,int w) const { return blockSize*w+k; }
public:
	// konstruktor
	CSimpleCompressedFrame(const IInfo& iinfo,int blockSize, char manipUID);

	CSimpleCompressedFrame(const CBlock& block);
	
	virtual ~CSimpleCompressedFrame();
	
	// Implementacja IFrame
	virtual const IInfo& GetInfo() const;

	// rozmiar ramki nieskompresowanej (bez UIDa)
	// (rozmiar po skompresowaniu mozna odczytac ze strumienia)
	virtual int GetSize() const;
	
	virtual CBitStream GetDataStream() const;
		
	virtual const GUID GetGUID() const { return guid; }
	virtual char GetUID() const { return guid.GetFrameUID(); }
		
	virtual void SetDataStream(const CBitStream& stream);

	virtual void SetCompressedDataStream(const CBitStream& compressedDataStream);
	
	virtual const CBitStream& GetCompressedDataStream() const;
	
	virtual CBitStream Serialize() const;
	
	virtual IEntropyCompressor* GetCompressor() const;

	virtual IPredictor* GetPredictor() const;

	// metody specyficzne dla CSimpleFrame
	int GetBlockSize() const;

	CSample* GetSamples(int chNum) const;

	virtual std::ostream& Print(std::ostream& ostr) const
	{
		ostr << "CSimpleCompressedFrame(UID= "<< (int)GetUID() <<") {"<<endl;
		ostr << "blockSize= "<<blockSize<<endl;
		ostr << info << endl;
		ostr << "Frame data: samples {"<<endl;
		if(samples == NULL) ostr << "NULL"<<endl;
		else PrintArray(ostr,samples,blockSize,16,4," ",16);
		ostr << endl;
		ostr << "}" << endl;
		ostr << "Compressed data: {"<<endl;
		if(compressed == NULL) ostr<<"NULL";
		else ostr<<*compressed;
		ostr<< endl;
		ostr << "}"<<endl;
		ostr << "};"<<endl;
		return ostr;
	}

	// konstruktor deserializujacy
	CSimpleCompressedFrame(const IInfo& info, const GUID guid,
			CBitStream& stream)
		: info(info), guid(guid)
	{
		SetCompressedDataStream(stream);
	}
};

#endif //CSIMPLECOMPRESSEDFRAME_H
