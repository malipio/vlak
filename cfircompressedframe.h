// $Id: cfircompressedframe.h,v 1.4 2006/01/11 18:56:29 wwasiak Exp $
#ifndef CFIRCOMPRESSEDFRAME_H
#define CFIRCOMPRESSEDFRAME_H
#include "uid.h"
#include "icompressedframe.h"
#include "csample.h"
#include "cblock.h"
#include "streamops.h"
#include <iostream>
//#include "cfirpredictor.h"

//TODO tymczasowo
#include "csimplecompressedframe.h"

class CFIRCompressedFrame; //TODO

using std::endl;

// UID = 2
class CFIRCompressedFrame : public ICompressedFrame
{
private:
	GUID guid;
	CSample *samples;
	const IInfo& info;
	int blockSize;
	CBitStream *data;
	CBitStream *compressed;
	CSample *startsamples;

	inline int _kw(int k,int w) const { return blockSize*w+k; } //TODO
public:
	// konstruktor
	CFIRCompressedFrame(const IInfo& iinfo,int blockSize, char manipUID);

	CFIRCompressedFrame(const CBlock& block);

	// konstruktor deserializujacy
        CFIRCompressedFrame(const IInfo& info, const GUID guid, CBitStream& stream);	
	
	virtual ~CFIRCompressedFrame();
	
	// Implementacja IFrame
	virtual const IInfo& GetInfo() const;

	// rozmiar ramki nieskompresowanej (bez UIDa)
	// (rozmiar po skompresowaniu mozna odczytac ze strumienia)
	virtual int GetSize() const;

	CExtSample* GetResiduals(char chNum);
	
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
		ostr << "CFIRCompressedFrame(UID= "<< (int)GetUID() <<") {"<<endl;
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

    friend class ICompressedFrame;
    friend class CFIRPredictor;
};

#endif //CFIRCOMPRESSEDFRAME_H
