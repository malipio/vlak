// $Id: cwaveletcompressedframe.cpp,v 1.4 2006/01/11 19:28:15 wwasiak Exp $
#include "cwaveletcompressedframe.h"
#include "cwaveletpredictor.h"
#include "ientropycompressor.h" // CSimpleCompressor
#include "RiceCoder/cricecoder.h"


// konstruktor
CWaveletCompressedFrame::CWaveletCompressedFrame(const IInfo& iinfo,int newBlockSize, 
		char manipUID) : 
	guid(CWAVELETCOMPRESSEDFRAME_UID,manipUID),
	info(iinfo), blockSize(newBlockSize), 
	data(NULL), compressed(NULL)
{
	if(blockSize != 0)
	{
		samples = new CSample[blockSize*info.GetNumOfChannels()];
	} else samples = NULL;
}

CWaveletCompressedFrame::CWaveletCompressedFrame(const CBlock& block) : 
	guid(CWAVELETCOMPRESSEDFRAME_UID,block.GetManipulatorUID()),
	info(block.GetInfo()), 
					      blockSize(block.GetBlockSize()),
					      data(NULL), compressed(NULL)
{
	samples = new CSample[blockSize*info.GetNumOfChannels()];
	// przepisujemy probki
	for(int ch = 0; ch < info.GetNumOfChannels(); ++ch)
	{
		CSample* ptr = block.GetSamples(ch);
		for(int i = 0; i< blockSize; ++i)
			samples[_kw(i,ch)]=ptr[i];
	}
}


//konstr. deserializujacy
CWaveletCompressedFrame::CWaveletCompressedFrame(const IInfo& iinfo,const GUID _guid, CBitStream& stream) :
	guid(_guid), info(iinfo), data(NULL), compressed(NULL)
{
//stream.GetNextNBytes((byte *)startsamples,6*sizeof(CSample));
unsigned short tmp;
stream.GetNextNBytes((byte *)&tmp, sizeof(unsigned short));
blockSize=tmp;
if(blockSize != 0)
{
	samples = new CSample[blockSize*info.GetNumOfChannels()];
} else samples = NULL;
SetCompressedDataStream(CBitStream(stream,false));
}


// TBD: wirtualny??
CWaveletCompressedFrame::~CWaveletCompressedFrame()
{
	if(samples != NULL) delete[] samples; 
	if(data != NULL) delete data;
	if(compressed != NULL) delete compressed;
}

const IInfo& CWaveletCompressedFrame::GetInfo() const 
{ 
	return info;
}



//TODO ta funkcja chyba nie jest wykorzystywana
int CWaveletCompressedFrame::GetSize() const // rozmiar ramki bez naglowka (?)
{ 
	return blockSize*info.GetNumOfChannels()*sizeof(CExtSample);
}

CExtSample* CWaveletCompressedFrame::GetResiduals(char chNum)
{
if (data==NULL) return NULL;
return ((CExtSample*)(data->GetStreamStart())) + chNum*blockSize;
}

CBitStream CWaveletCompressedFrame::GetDataStream() const
{
	return CBitStream(*data,true);
}
	
void CWaveletCompressedFrame::SetDataStream(const CBitStream& stream) //czyta od zadanego miejsca a nie caly stream
{
if (data != NULL) delete data;
data = new CBitStream(stream,false);
//blockSize = stream.Size()/(info.GetNumOfChannels()*sizeof(int)); // sygnal residualny to cextsample
//if(samples != NULL) delete[] samples;
//samples = new CSample[blockSize*info.GetNumOfChannels()];
}

void CWaveletCompressedFrame::SetCompressedDataStream(const CBitStream& compressedDataStream)
{
	if(compressed != NULL) delete compressed;
	compressed = new CBitStream(compressedDataStream,false);
	// !! wykonujemy kopie strumienia od aktualnej pozycji w pliku
}

const CBitStream& CWaveletCompressedFrame::GetCompressedDataStream() const
{
	//TODO assert(compressed != NULL)
	return *compressed;
}

CBitStream CWaveletCompressedFrame::Serialize() const
{
	CBitStream s(compressed->Size() + sizeof(unsigned short) );
	// tutaj zapisujemy dane skompresowane
	//TODO assert(compressed != NULL)
	unsigned short tmp=(unsigned short) blockSize;
	s.Write(&tmp, sizeof(unsigned short));
	s.Write(compressed->GetBytes(),compressed->Size());
	return s;
}

IEntropyCompressor* CWaveletCompressedFrame::GetCompressor() const
{
	return new CRiceCoder();
//	return new CSimpleCompressor();
}

IPredictor* CWaveletCompressedFrame::GetPredictor() const
{
	return new CWaveletPredictor();
}

int CWaveletCompressedFrame::GetBlockSize() const
{
	return blockSize;
}

CSample* CWaveletCompressedFrame::GetSamples(int chNum) const
{
	//TODO
	return &(samples[_kw(0,chNum)]);
}
