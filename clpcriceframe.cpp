// $Id: clpcriceframe.cpp,v 1.4 2005/11/26 19:10:58 pmalinow Exp $
#include "clpcriceframe.h"
#include "clpcpredictor.h"
#include "ientropycompressor.h" // poki co uzywamy CSimpleCompressora
#include "RiceCoder/cricecoder.h"
#include <limits.h>

using std::cerr;
using std::endl;
//#define DEBUG_SERIALIZE


CLPCRiceFrame::CLPCRiceFrame(const IInfo& iinfo, int blockSize, char manipUID) : 
	info(iinfo), blockSize(blockSize), 
	guid(CLPCRICEFRAME_UID,manipUID, CRICECODER_UID) // uzywamy rice'a domyslnie
{
	Init();
}

void CLPCRiceFrame::InitArrays(char order[], bool initResidual) // pozostale arg. sa brane z IInfo
{
	assert(coefficients == NULL);
	assert(residualPart == NULL);
	assert(startSamples == NULL);
	assert(CLPCRiceFrame::order == NULL);
	// dla wygody:
	char chNum = info.GetNumOfChannels();
	CLPCRiceFrame::order = new char[chNum];
	memcpy(CLPCRiceFrame::order,order,chNum*sizeof(char));
	
	coefficients = new float *[chNum];
	if(initResidual) residualPart = new CExtSample *[chNum];
	else residualPart = NULL;
	startSamples = new CSample *[chNum];
	for(int i = 0; i < chNum; ++i)
	{
		if(order[i] == 0)
		{
			coefficients[i] = NULL;
			if(initResidual) residualPart[i] = NULL;
			startSamples[i] = new CSample[1]; // wartosc sygnalu
		} else 
		{
			//assert(order[i]>0);
			coefficients[i] = new float[order[i]];
			
			if(initResidual)
			{
				assert(blockSize-order[i] > 0);
				residualPart[i] = new CExtSample[blockSize-order[i]];
			}
			startSamples[i] = new CSample[order[i]];
		}
	}
}

// implementacja IFrame

CLPCRiceFrame::~CLPCRiceFrame()
{

	for(int chNum = 0; chNum < info.GetNumOfChannels(); ++chNum)
	{
		if(coefficients != NULL && coefficients[chNum] != NULL) 
			delete[] coefficients[chNum];
		if(residualPart != NULL && residualPart[chNum] != NULL) 
		{
		//	std::cout << "ch= "<<chNum<<" "<<residualPart[chNum]<<std::endl;
			delete[] residualPart[chNum];
		}
		if(startSamples != NULL && startSamples[chNum] != NULL) 
			delete[] startSamples[chNum];
	}
	if(coefficients != NULL) delete[] coefficients;
	if(residualPart != NULL) delete[] residualPart;
	if(startSamples != NULL) delete[] startSamples;
	if(order != NULL) delete[] order;
	// ten strumien sami alokowalismy
	if(compressed != NULL) delete compressed;
}

const IInfo& CLPCRiceFrame::GetInfo() const { return info; }

int CLPCRiceFrame::GetSize() const { return -1; } // TODO

CBitStream CLPCRiceFrame::GetDataStream() const // TODO TBD: trzeba dla kazdego kanalu osobno?
{ 
	int size  = 0;
	for(int chNum = 0; chNum < info.GetNumOfChannels(); ++chNum)
	{
		// liczymy rozmiar potrzebnego strumienia...
		if(order[chNum] == 0) continue; // brak sygnalu residualnego
		size += blockSize - order[chNum];
	}

	// taki hack, bo inaczej nie uda nam sie zwolnic tej pamieci :(
	CBitStream tmpStream(size*sizeof(CExtSample)); // size moze byc = 0
	CExtSample * flatResidual = (CExtSample *)tmpStream.GetBytes();
	assert(flatResidual != NULL); // moze tak byc dla order = [ 0 ... 0 ]
	
	for(int chNum = 0, j = 0; chNum < info.GetNumOfChannels(); ++chNum)
	{
		if(residualPart[chNum] == NULL) continue; // order[chNum] == 0
		
		memcpy(flatResidual+j,residualPart[chNum], 
				(blockSize-order[chNum])*sizeof(CExtSample));
		j += (blockSize-order[chNum]);
	}
	return CBitStream(tmpStream,true); // full copy
	
}
	
// metoda ta bierze jako argument strumien danych
// i ustawia swoje odpowiednie pola na jego podstawie
// UWAGA: nie ma to nic wspolnego z Serialize/Deserialize!!
void CLPCRiceFrame::SetDataStream(const CBitStream& stream)
{
	// stream powinien zawierac sygnal residualny
	assert(residualPart == NULL); // innych sytuacji nie chce mi sie obsluzyc
	assert(order != NULL);
	CExtSample *tmp = reinterpret_cast<CExtSample *>(stream.GetBytes());

	// residualSize = sizeof(CExtSample)*
	// 	NumOfChannels(blockSize-order[chNum]) w przyblizeniu...
	//int i = 0;
	//int orderSum = 0;
	//for(int chNum = 0; chNum < info.GetNumOfChannels(); ++chNum)
	//	if(order[chNum] != 0)
	//	{
	//		i++;
	//		orderSum += sizeof(CExtSample)*order[chNum];
	//	}
	//blockSize = (stream.Size()+orderSum)/sizeof(CExtSample);
	//blockSize /= i; 
	// niewazne jak, wazne ze dziala takie obliczenie
	
#ifdef DEBUG_SERIALIZE
	cerr << "blockSize = "<<blockSize<<endl;
#endif

	// przydzielamy pamiec
	residualPart = new CExtSample *[info.GetNumOfChannels()];
	int pos = 0;
	for(int chNum = 0; chNum < info.GetNumOfChannels(); ++chNum)
	{
		if(order[chNum] == 0) 
		{ 
			residualPart[chNum] = NULL; 
			continue; 
		}
		residualPart[chNum] = new CExtSample[blockSize-order[chNum]];
		memcpy(residualPart[chNum],tmp+pos,
				(blockSize-order[chNum])*sizeof(CExtSample));
		pos += blockSize-order[chNum];
		
	}
}
	
// metoda zwraca wskaznik na obiekt predyktora uzytego w tej ramce
IPredictor* CLPCRiceFrame::GetPredictor() const 
{ 
	return new CLPCPredictor(); 
}

// metoda wypisujaca informacje o klasie
std::ostream& CLPCRiceFrame::Print(std::ostream& ostr) const
{
	ostr << "CLPCRiceFrame(UID = "<<(short)GetUID()<<") {"<<std::endl;
	ostr << "GUID = "<<GetGUID()<<endl;
	ostr << info;
	ostr << "(source) blockSize = "<<blockSize<<std::endl;
	ostr << "order {"<<std::endl;
	if(order == NULL) ostr<<"NULL"<<std::endl;
	else PrintArray(ostr,(byte *)order,info.GetNumOfChannels(),10,3," ",
			info.GetNumOfChannels(),' ');
	ostr << "}" << std::endl;
	
	ostr << "coefficients {" <<std::endl;
	if(coefficients == NULL) ostr << "NULL"<<std::endl;
	else 
	{
		for(char chNum = 0; chNum < info.GetNumOfChannels(); ++chNum)
		{
			ostr << "channel "<<(short)chNum<< " {"<< std::endl;
			if(order[chNum] == 0) // CONST FRAME
				ostr << "NULL - const frame" << std::endl;
			else
			{
				if(coefficients[chNum] == NULL)
					ostr << "NULL" << std::endl;
				else
				PrintArray(ostr,coefficients[chNum],
						order[chNum],10,10," ",order[chNum],' ');
			}
			ostr << "}"<<std::endl;
		}
	}
	ostr << "}" <<std::endl;

	ostr << "startSamples {" <<std::endl;
	if(startSamples == NULL) ostr << "NULL"<<std::endl;
	else 
	{
		for(char chNum = 0; chNum < info.GetNumOfChannels(); ++chNum)
		{
			ostr << "channel "<<(short)chNum<< " {"<< std::endl;
			if(order[chNum] == 0) // CONST FRAME
				ostr << startSamples[chNum][0] << std::endl;
			else
			{
				if(startSamples[chNum] == NULL)
					ostr << "NULL" << std::endl;
				else
				PrintArray(ostr,startSamples[chNum],
						order[chNum],10,10," ",order[chNum],' ');
			}
			ostr << "}"<<std::endl;
		}
	}
	ostr << "}" <<std::endl;

	ostr << "residualPart {" <<std::endl;
	if(residualPart == NULL) ostr << "NULL"<<std::endl;
	else 
	{
		for(char chNum = 0; chNum < info.GetNumOfChannels(); ++chNum)
		{
			ostr << "channel "<<(short)chNum<< " {"<< std::endl;
			if(order[chNum] == 0) // CONST FRAME
				ostr << "NULL - const frame" << std::endl;
			else
			{
				if(residualPart[chNum] == NULL)
					ostr << "NULL" << std::endl;
				else
				PrintArray(ostr,residualPart[chNum],
						blockSize-order[chNum],10,7," ",10,' ');
			}
			ostr << "}"<<std::endl;
		}
	}
	ostr << "}" <<std::endl;
	
	ostr << "compressed {" << std::endl;
	if(compressed == NULL) ostr << "NULL"<<std::endl;
	else ostr << *compressed;
	ostr << "}"<<std::endl;
	ostr << "};"<<std::endl;
	return ostr;
	
}

// implementacja ICompressedFrame

void CLPCRiceFrame::SetCompressedDataStream (const CBitStream& compressedDataStream)
{
	assert(compressed == NULL);
	//moze sie zdarzyc ze compressedDataStream.Size() == 0 -
	//wtedy po prostu nie ma danych skompresowanych
	compressed = new CBitStream(compressedDataStream,true); // full copy
	// TODO: reset()?
}

// pobiera skompresowany strumien w celu dekompresji
const CBitStream& CLPCRiceFrame::GetCompressedDataStream() const
{
	assert(compressed != NULL);
	return *compressed;
}

// serializuje obiekt do strumienia
CBitStream CLPCRiceFrame::Serialize() const
{
	assert(compressed != NULL); // serializacja tylko po kompresji
	// scenariusz
	// 00. wylicz rozmiar strumienia i go utworz
	
	// UWAGA: uid zapisuje teraz CVLAKFile::WriteFrame
	//char uid = GetUID();
	int coeffSize = 0, samplesSize = 0;
	for(int chNum = 0; chNum < info.GetNumOfChannels(); ++chNum)
	{
		coeffSize += order[chNum];
		samplesSize += (order[chNum] == 0) ? 1 : order[chNum];
	}
	int totalSize = info.GetNumOfChannels()*sizeof(char) + sizeof(unsigned short) +// order[]
		coeffSize*sizeof(float)+samplesSize*sizeof(CSample) +
		compressed->Size();
#ifdef DEBUG_SERIALIZE
	cerr << "totalSize = "<<totalSize<<endl;
#endif
	CBitStream stream(totalSize);
	// 0. zapisz uid
	//stream.Write(uid);
	// 1. zapisz blockSize (konieczne dla poprawnego odtworzenia ramki const)
	assert(blockSize <= USHRT_MAX); // zapisujemy jako ushort
	unsigned short short_blockSize = blockSize;
	stream.Write(&short_blockSize,sizeof(unsigned short)); 
	// 2. zapisz order[]
	stream.Write(order,info.GetNumOfChannels()*sizeof(char));
	// 3. zapisz coefficients[][]
	// tablice dwuwymiarowa trudniej sie zapisuje :(
	for(int chNum = 0; chNum < info.GetNumOfChannels(); ++chNum)
		if(order[chNum] != 0)
			stream.Write(coefficients[chNum],order[chNum]*sizeof(float));
	// 4. zapisz startSamples[][]
	for(int chNum = 0; chNum < info.GetNumOfChannels(); ++chNum)
		if(order[chNum] != 0)
			stream.Write(startSamples[chNum],order[chNum]*sizeof(CSample));
		else stream.Write(startSamples[chNum],1*sizeof(CSample));
	// 5. zapisz strumien compressed
	stream.Write(compressed->GetBytes(),compressed->Size());
#ifdef DEBUG_SERIALIZE
	cerr << stream << endl;
#endif
	return CBitStream(stream,true); // full copy
}

// metoda zwraca wskaznik na obiekt ktory skompresowal ta ramke
IEntropyCompressor* CLPCRiceFrame::GetCompressor() const
{
	return new CRiceCoder();
	//return new CSimpleCompressor();
}

// konstruktor deserializujacy
CLPCRiceFrame::CLPCRiceFrame(const IInfo& info, const GUID guid,CBitStream& stream) 
	: info(info), guid(guid)
{

	Init();
	blockSize = 0; // jeszcze nieznany
	// blockSize mozna zazwyczaj wydedukowac z rozmiaru sygnalu residualnego
	// jednak w przypadku gdy ramka jest CONST nie ma tego sygnalu!
	
	// scenariusz
	// 1. odczytaj blockSize
	unsigned short short_block;
	stream.GetNextNBytes((byte *)&short_block,sizeof(unsigned short));
	blockSize = short_block;
	// 2. odczytaj order[]
	char * tmpOrder = new char[info.GetNumOfChannels()];
	stream.GetNextNBytes((byte *)tmpOrder,info.GetNumOfChannels()*sizeof(char));
	InitArrays(tmpOrder,false); // przydzielamy pamiec za wyjatkiem residualPart
	// oraz ustawiamy order odpowiednio
	delete[] tmpOrder;
	// 2. odczytaj coefficients[][]
	for(int chNum = 0; chNum < info.GetNumOfChannels(); ++chNum)
		if(order[chNum] != 0)
			stream.GetNextNBytes((byte *)(coefficients[chNum]),
					order[chNum]*sizeof(float));
	// 3. odczytaj startSamples[][]
	for(int chNum = 0; chNum < info.GetNumOfChannels(); ++chNum)
		if(order[chNum] != 0)
			stream.GetNextNBytes((byte *)startSamples[chNum],
					order[chNum]*sizeof(CSample));
		else 
			stream.GetNextNBytes((byte *)startSamples[chNum],
					1*sizeof(CSample));
	// 4. odczytaj strumien compressed
	// TODO: fajnie by bylo miec odpowiednia metode w CBitStream
	// a poki co jest ugly hack - zajebiscie powolny
	SetCompressedDataStream(CBitStream(stream,false)); // NOT full copy
	
}

