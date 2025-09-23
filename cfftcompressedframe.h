// $Id: cfftcompressedframe.h,v 1.12 2006/01/12 03:04:05 kjamroz Exp $
#ifndef CFFTCOMPRESSEDFRAME_H
#define CFFTCOMPRESSEDFRAME_H
#include "uid.h"
#include "icompressedframe.h"
#include "csample.h"
#include "cblock.h"
#include "streamops.h"
#include <iostream>
#include <vector>

using std::vector;
using std::endl;

class CFFTCompressedFrame;

class CFFTChannelFrame
{
public:
	CFFTChannelFrame(CFFTCompressedFrame& aFFTFrame);
	~CFFTChannelFrame();

	/// Copies stored samples to aSamples
	void GetSamples(CExtSample* aSamples, int aCount) const;
	
	const CExtSample* Samples() const
	{
		return iResidual;
	}

	/// Copies aSamples to stored samples
	void SetSamples(const CExtSample* aSamples, int aCount);

	/// Sets compressed spectrum, takes ownership
	void SetSpectrum(CBitStream* spectrumstream);

	/// Gets spectrum
	CBitStream* Spectrum();

	const CBitStream* Spectrum() const;

	void SetConstantComponent(float aConstant);

private:
	CFFTCompressedFrame& iFFTFrame;

	/// After compression - residual signal
	CExtSample *iResidual;

	/// Encoded spectrum
	CBitStream *iSpectrum;

	//compressed data:
	float iConstantComponent;
};

// UID = 4
class CFFTCompressedFrame : public ICompressedFrame
{
public:
	// Constructor
	CFFTCompressedFrame(const IInfo& iinfo,int blockSize, char manipUID);

	CFFTCompressedFrame(const CBlock& block);
	
	// Deserializing constructor
	CFFTCompressedFrame(const IInfo& info, const GUID guid, CBitStream& stream);

	virtual ~CFFTCompressedFrame();

	CBitStream& DecompressedDataStream() const
	{
		assert(iDecompressedDataStream!=NULL);
		//frame is being decompressed
		return *iDecompressedDataStream;
	}

	//predictor parameters
	float QFACTOR;  //integer
	float MINRELAMPL;

public:  // from IFrame
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

	// metody specyficzne dla CFFTCompressedFrame
	int GetBlockSize() const;

	CSample* GetSamples(int chNum) const;

	virtual std::ostream& Print(std::ostream& ostr) const;

	CFFTChannelFrame& ChannelFrame(int aChannelNum);
	const CFFTChannelFrame& ChannelFrame(int aChannelNum) const;

private:

	inline int _kw(int k,int w) const { return blockSize*w+k; } //TODO - remove it

	/// Samples contained in this frame
	CSample *samples;

	const IInfo& info;
	int blockSize;  //number of samples in this frame
	CBitStream *compressed;
	GUID guid;

	int adjustedBlockSize;
	CFFTChannelFrame** iChannelFrames;

	void CreateChannelFrames();

	CBitStream *iDecompressedDataStream;
};

inline CFFTChannelFrame& CFFTCompressedFrame::ChannelFrame(int aChannelNum)
{
	return *(iChannelFrames[aChannelNum]);
}
inline const CFFTChannelFrame& CFFTCompressedFrame::ChannelFrame(int aChannelNum) const
{
	return *(iChannelFrames[aChannelNum]);
}

#endif //CFFTCOMPRESSEDFRAME_H
