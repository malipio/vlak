// $Id: cfftcompressedframe.cpp,v 1.17 2006/01/12 03:48:37 kjamroz Exp $
#include <math.h>
#include "fft.h"

#include "cfftcompressedframe.h"
#include "cfftpredictor.h"

#include "ientropycompressor.h" // CSimpleCompressor
#include "RiceCoder/cricecoder.h"

/////////////////////////////////////////////////
// CFFTChannelFrame class
/////////////////////////////////////////////////

CFFTChannelFrame::CFFTChannelFrame(CFFTCompressedFrame& aFFTFrame) :
iFFTFrame(aFFTFrame),
iResidual(NULL),
iSpectrum(NULL), 
iConstantComponent(0)
{
}

CFFTChannelFrame::~CFFTChannelFrame()
{
	delete [] iResidual;
	delete iSpectrum;
}

void CFFTChannelFrame::GetSamples(CExtSample* aSamples, int aCount) const
{
	assert(iResidual!=NULL);
	assert(aCount == iFFTFrame.GetBlockSize());

	memcpy(aSamples, iResidual, aCount*sizeof(CExtSample));
}

void CFFTChannelFrame::SetSamples(const CExtSample* aSamples, int aCount)
{
	assert(iResidual==NULL);
	assert(aCount == iFFTFrame.GetBlockSize());

	iResidual = new CExtSample[aCount];
	memcpy(iResidual, aSamples, aCount*sizeof(CExtSample));
}

void CFFTChannelFrame::SetSpectrum(CBitStream* spectrumstream)
{
	delete iSpectrum;
	iSpectrum = spectrumstream;
}

CBitStream* CFFTChannelFrame::Spectrum()
{
	return iSpectrum;
}
const CBitStream* CFFTChannelFrame::Spectrum() const
{
	return iSpectrum;
}

void CFFTChannelFrame::SetConstantComponent(float aConstant)
{
	iConstantComponent = aConstant;
}

/////////////////////////////////////////////////
// CFFTCompressedFrame class
/////////////////////////////////////////////////

CFFTCompressedFrame::CFFTCompressedFrame(const IInfo& iinfo,
		int blockSize, char manipUID) : 
	info(iinfo), compressed(NULL), guid(CFFTCOMPRESSEDFRAME_UID,manipUID),
	iChannelFrames(NULL), iDecompressedDataStream(NULL)
{
	if(blockSize != 0)
	{
		samples = new CSample[blockSize*info.GetNumOfChannels()];
	}
	else
	{
		samples = NULL;
	}
	
	CFFTCompressedFrame::blockSize=blockSize;

	CreateChannelFrames();
}

CFFTCompressedFrame::CFFTCompressedFrame(const CBlock& block) 
	: info(block.GetInfo()), 
	blockSize(block.GetBlockSize()),
	compressed(NULL),
	guid(CFFTCOMPRESSEDFRAME_UID, block.GetManipulatorUID()),
	iChannelFrames(NULL), iDecompressedDataStream(NULL)
{
	samples = new CSample[blockSize*info.GetNumOfChannels()];
	// przepisujemy probki
	for(int ch = 0; ch < info.GetNumOfChannels(); ++ch)
	{
		CSample* ptr = block.GetSamples(ch);
		//for(int i = 0; i< blockSize; ++i)
		//	samples[_kw(i,ch)]=ptr[i];
		memcpy(samples + ch*blockSize, ptr, sizeof(CSample) * blockSize);
	}

	CreateChannelFrames();
}

CFFTCompressedFrame::CFFTCompressedFrame(const IInfo& info, const GUID guid, CBitStream& stream)
	: info(info), blockSize(0),guid(guid), compressed(NULL), samples(NULL),
	iChannelFrames(NULL), iDecompressedDataStream(NULL)
{
	CreateChannelFrames();

	//read block size
	unsigned short sblocksize;
	stream.GetNextNBytes((byte*)&sblocksize, 2);
	blockSize = sblocksize;

	/// fft predictor params
    unsigned char qfactorb;
	stream.GetNextNBytes((byte*)&qfactorb, 1);
	QFACTOR = qfactorb;

	stream.GetNextNBytes((byte*)&MINRELAMPL, sizeof(float));

	//SetCompressedDataStream(CBitStream(stream,false));
	compressed = new CBitStream(stream, false);
}

CFFTCompressedFrame::~CFFTCompressedFrame()
{
	delete[] samples; 
	delete compressed;
	delete iDecompressedDataStream;

	for(int i=0;i<info.GetNumOfChannels();++i)
	{
		delete iChannelFrames[i];
	}
	delete [] iChannelFrames;
}

void CFFTCompressedFrame::CreateChannelFrames()
{
	iChannelFrames = new CFFTChannelFrame* [info.GetNumOfChannels()];
	for(int i=0;i<info.GetNumOfChannels();++i)
	{
		iChannelFrames[i] = new CFFTChannelFrame(*this);
	}
}

const IInfo& CFFTCompressedFrame::GetInfo() const 
{ 
	return info;
}

int CFFTCompressedFrame::GetSize() const // rozmiar ramki bez naglowka (?)
{ 
	//TODO: real size
	return blockSize*info.GetNumOfChannels()*sizeof(CSample); 
}
	
CBitStream CFFTCompressedFrame::GetDataStream() const
{
	int streamlen = info.GetNumOfChannels()*blockSize*sizeof(CExtSample);  //residual signal
	for(int i=0;i<info.GetNumOfChannels();++i)
		streamlen += ChannelFrame(i).Spectrum()->Size();
	
	//create stream
	CBitStream bs(streamlen);

	//store spectrums
	for(int i=0;i<info.GetNumOfChannels();++i)
	{
		//cout << "write spectrum " << i << " nbytes=" << ChannelFrame(i).Spectrum()->Size() << endl;
		bs.Write(ChannelFrame(i).Spectrum()->GetBytes(), ChannelFrame(i).Spectrum()->Size());
	}

	//store residual signal
	for(int i=0;i<info.GetNumOfChannels();++i)
	{
		//cout << "write residual " << i << " bytes=" << blockSize*sizeof(CExtSample)<< endl;
		//TODO: it is temporary
		bs.Write(const_cast<CExtSample*>(iChannelFrames[i]->Samples()),	blockSize*sizeof(CExtSample));
	}
	
	bs.Reset();
	
	return bs;
}
	
void CFFTCompressedFrame::SetDataStream(const CBitStream& stream)
{
	// dostajemy strumien danych taki jak robimy w GetDataStream

	//stream is decoded in the predictor
	assert(iDecompressedDataStream==NULL);
	
	//cout <<"set data stream: len=" << stream.Size()<<endl;

	iDecompressedDataStream = new CBitStream(stream, true);	

//	cout <<"iDecompressedDataStream: len=" << iDecompressedDataStream->Size() <<endl;
}

void CFFTCompressedFrame::SetCompressedDataStream(const CBitStream& compressedDataStream)
{
	//cout << "set compressed stream len=" <<  compressedDataStream.Size() << endl;
	
	assert(compressed == NULL);
	compressed = new CBitStream(compressedDataStream,true); 
	// !! wykonujemy kopie strumienia od aktualnej pozycji w pliku
}

const CBitStream& CFFTCompressedFrame::GetCompressedDataStream() const
{
	assert(compressed != NULL);
	return *compressed;
}

CBitStream CFFTCompressedFrame::Serialize() const
{
	// serializacja wszedzie wyglada podobnie
	// TODO: kolejne metody dla CBitStream'a
	//char uid = GetUID();
	int size = 4+5+compressed->Size();
	//for(int i=0;i<info.GetNumOfChannels();++i)
	//	size += ChannelFrame(i).Spectrum()->Size();
	
	CBitStream s(size);

	// tutaj zapisujemy dane ktore nie sa kompresowane
	
	//blocksize has to be stored
	unsigned short sblocksize = (unsigned short)blockSize;
	s.Write(&sblocksize,2);

	/// fft predictor params
    unsigned char qfactorb = (unsigned char)QFACTOR;
	s.Write((byte*)&qfactorb, 1);
	s.Write((byte*)&MINRELAMPL, sizeof(float));

	// ........................
	//assert(compressed != NULL)
	// tutaj zapisujemy dane skompresowane
	s.Write(compressed->GetBytes(),compressed->Size());

	return s;
}

IEntropyCompressor* CFFTCompressedFrame::GetCompressor() const
{
//	return new CSimpleCompressor();
	return new CRiceCoder();
}

IPredictor* CFFTCompressedFrame::GetPredictor() const
{
	return new CFFTPredictor();
}

int CFFTCompressedFrame::GetBlockSize() const
{
	return blockSize;
}

CSample* CFFTCompressedFrame::GetSamples(int chNum) const
{
	return &(samples[_kw(0,chNum)]);
}

std::ostream& CFFTCompressedFrame::Print(std::ostream& ostr) const
{
	ostr << "CFFTCompressedFrame(UID= "<< (int)GetUID() <<") {"<<endl;
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



