// $Id: cfircompressedframe.cpp,v 1.8 2006/01/11 19:28:15 wwasiak Exp $
#include "cfircompressedframe.h"
#include "cfirpredictor.h" // CFIRPredictor
#include "ientropycompressor.h" // CSimpleCompressor
#include "RiceCoder/cricecoder.h"


// konstruktor
CFIRCompressedFrame::CFIRCompressedFrame(const IInfo& iinfo,int newBlockSize, 
		char manipUID) : 
	guid(CFIRCOMPRESSEDFRAME_UID,manipUID),
	info(iinfo), blockSize(newBlockSize), 
	data(NULL), compressed(NULL), startsamples(NULL)
{
	if(blockSize != 0)
	{
		samples = new CSample[blockSize*info.GetNumOfChannels()];
	} else samples = NULL;
	startsamples=new CSample[6];
}

CFIRCompressedFrame::CFIRCompressedFrame(const CBlock& block) : 
	guid(CFIRCOMPRESSEDFRAME_UID,block.GetManipulatorUID()),
	info(block.GetInfo()), 
					      blockSize(block.GetBlockSize()),
					      data(NULL), compressed(NULL), startsamples(NULL)
{
	samples = new CSample[blockSize*info.GetNumOfChannels()];
	// przepisujemy probki
	for(int ch = 0; ch < info.GetNumOfChannels(); ++ch)
	{
		CSample* ptr = block.GetSamples(ch);
		for(int i = 0; i< blockSize; ++i)
			samples[_kw(i,ch)]=ptr[i];
	}
	startsamples=new CSample[6];
}


//konstr. deserializujacy
CFIRCompressedFrame::CFIRCompressedFrame(const IInfo& iinfo,const GUID _guid, CBitStream& stream) :
	guid(_guid), samples(NULL), info(iinfo), blockSize(0), data(NULL), compressed(NULL)
{
startsamples=new CSample[6];
stream.GetNextNBytes((byte *)startsamples,6*sizeof(CSample));
SetCompressedDataStream(CBitStream(stream,false));
}


// TBD: wirtualny??
CFIRCompressedFrame::~CFIRCompressedFrame()
{
	if(samples != NULL) delete[] samples; 
	if(data != NULL) delete data;
	if(compressed != NULL) delete compressed;
	if(startsamples != NULL) delete[] startsamples;
}

const IInfo& CFIRCompressedFrame::GetInfo() const 
{ 
	return info;
}



//TODO odtad w dol powinienem to pozmieniac
int CFIRCompressedFrame::GetSize() const // rozmiar ramki bez naglowka (?)
{ 
	return blockSize*info.GetNumOfChannels()*sizeof(CExtSample)+3;
}

CExtSample* CFIRCompressedFrame::GetResiduals(char chNum)
{
if (data==NULL) return NULL;
return ((CExtSample*)(data->GetStreamStart())) + chNum*(blockSize-3);
}

	
CBitStream CFIRCompressedFrame::GetDataStream() const
{
	// TODO: mozliwe ze konieczna bedzie fizyczna kopia danych
	//TODO assert data!=NULL
//	if (data==NULL)
//	    {
//	    printf("Data==NUL!!\n");
//	    exit(1);
//	    }

//data->Reset();
//return *data;

//TODO
return CBitStream(*data,true);
//return CBitStream(samples,GetSize());
}
	
void CFIRCompressedFrame::SetDataStream(const CBitStream& stream) //czyta od zadanego miejsca a nie caly stream
{
	//TODO - zmienic to
	// dostajemy strumien danych taki jak robimy w GetDataStream

	// assert(stream.Size() == GetSize())
//	unsigned char * data = (const_cast<CBitStream&>(stream)).GetBytes(); 
//	if(samples == NULL)
//	{
//		blockSize = stream.Size()/(info.GetNumOfChannels()*sizeof(CSample)); // hmm
//	}
//	// kopiujemy dane do naszej struktury 
//	memcpy(samples,data,stream.Size());

if (data != NULL) delete data;
data = new CBitStream(stream,false);
blockSize = stream.Size()/(info.GetNumOfChannels()*sizeof(CExtSample)) + 3; // sygnal residualny to cextsample | dodaje 3 sample ktore sa przechowywane inaczej
if(samples != NULL) delete[] samples;
samples = new CSample[blockSize*info.GetNumOfChannels()];
}

void CFIRCompressedFrame::SetCompressedDataStream(const CBitStream& compressedDataStream)
{
	if(compressed != NULL) delete compressed;
	compressed = new CBitStream(compressedDataStream,false);
	// !! wykonujemy kopie strumienia od aktualnej pozycji w pliku
}

const CBitStream& CFIRCompressedFrame::GetCompressedDataStream() const
{
	// assert(compressed != NULL)
	return *compressed;
}

CBitStream CFIRCompressedFrame::Serialize() const
{
	// serializacja wszedzie wyglada podobnie
	// TODO: kolejne metody dla CBitStream'a
	//char uid = GetUID();
	int size = 6*sizeof(CSample)+compressed->Size();
	CBitStream s(size);
	// tutaj zapisujemy dane ktore nie sa kompresowane
	//s.Write(uid);
	s.Write((byte *)startsamples,6*sizeof(CSample));
	//assert(compressed != NULL)
	// tutaj zapisujemy dane skompresowane
	s.Write(compressed->GetBytes(),compressed->Size());
	return s;
}

IEntropyCompressor* CFIRCompressedFrame::GetCompressor() const
{
	// TODO: podebugowac RiceCoder'a
	return new CRiceCoder();//new CSimpleCompressor(); //TODO
}

IPredictor* CFIRCompressedFrame::GetPredictor() const
{
	return new CFIRPredictor();
}

int CFIRCompressedFrame::GetBlockSize() const
{
	return blockSize;
}

CSample* CFIRCompressedFrame::GetSamples(int chNum) const
{
	//TODO
	return &(samples[_kw(0,chNum)]);
}
