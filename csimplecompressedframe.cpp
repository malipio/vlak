// $Id: csimplecompressedframe.cpp,v 1.4 2005/11/26 19:10:58 pmalinow Exp $
#include "csimplecompressedframe.h"
#include "ipredictor.h" // CSimplePredictor
#include "ientropycompressor.h" // CSimpleCompressor

// konstruktor
CSimpleCompressedFrame::CSimpleCompressedFrame(const IInfo& iinfo,
		int blockSize, char manipUID) : 
	info(iinfo), compressed(NULL), guid(CSIMPLECOMPRESSEDFRAME_UID,manipUID)
{
	if(blockSize != 0)
	{
		samples = new CSample[blockSize*info.GetNumOfChannels()];
	} else samples = NULL;
	
	CSimpleCompressedFrame::blockSize=blockSize;
}

CSimpleCompressedFrame::CSimpleCompressedFrame(const CBlock& block) : info(block.GetInfo()), 
					      blockSize(block.GetBlockSize()),
					      compressed(NULL),
					      guid(CSIMPLECOMPRESSEDFRAME_UID,
							      block.GetManipulatorUID())
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

// TBD: wirtualny??
CSimpleCompressedFrame::~CSimpleCompressedFrame()
{
	if(samples != NULL) delete[] samples; 
	if(compressed != NULL) delete compressed;
}

const IInfo& CSimpleCompressedFrame::GetInfo() const 
{ 
	return info;
}

int CSimpleCompressedFrame::GetSize() const // rozmiar ramki bez naglowka (?)
{ 
	return blockSize*info.GetNumOfChannels()*sizeof(CSample); 
}
	
CBitStream CSimpleCompressedFrame::GetDataStream() const
{
	// TODO: mozliwe ze konieczna bedzie fizyczna kopia danych
	return CBitStream(samples,GetSize());
}
	
void CSimpleCompressedFrame::SetDataStream(const CBitStream& stream)
{
	// dostajemy strumien danych taki jak robimy w GetDataStream

	unsigned char * data = stream.GetBytes(); 
	assert(samples == NULL);

	blockSize = stream.Size()/(info.GetNumOfChannels()*sizeof(CSample)); // hmm
	samples = new CSample[blockSize*info.GetNumOfChannels()];

	// kopiujemy dane do naszej struktury 
	memcpy(samples,data,stream.Size());
	
}

void CSimpleCompressedFrame::SetCompressedDataStream(const CBitStream& compressedDataStream)
{
	assert(compressed == NULL);
	compressed = new CBitStream(compressedDataStream,false); 
	// !! wykonujemy kopie strumienia od aktualnej pozycji w pliku
}

const CBitStream& CSimpleCompressedFrame::GetCompressedDataStream() const
{
	assert(compressed != NULL);
	return *compressed;
}

CBitStream CSimpleCompressedFrame::Serialize() const
{
	// serializacja wszedzie wyglada podobnie
	// TODO: kolejne metody dla CBitStream'a
	//char uid = GetUID();
	//int size = compressed->Size();
	CBitStream s(compressed->Size());
	//s.Write(uid);
	// tutaj zapisujemy dane ktore nie sa kompresowane
	// ........................
	//assert(compressed != NULL)
	// tutaj zapisujemy dane skompresowane
	s.Write(compressed->GetBytes(),compressed->Size());
	return s;
}

IEntropyCompressor* CSimpleCompressedFrame::GetCompressor() const
{
	return new CSimpleCompressor();
}

IPredictor* CSimpleCompressedFrame::GetPredictor() const
{
	return new CSimplePredictor();
}

int CSimpleCompressedFrame::GetBlockSize() const
{
	return blockSize;
}

CSample* CSimpleCompressedFrame::GetSamples(int chNum) const
{
	return &(samples[_kw(0,chNum)]);
}


