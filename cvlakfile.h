// $Id: cvlakfile.h,v 1.7 2005/11/26 19:10:58 pmalinow Exp $
#ifndef CVLAKFILE_H
#define CVLAKFILE_H
#include <iostream>
#include <string>
#include "cvlakheader.h"
#include "icompressedframe.h"
#include "File.h" // klasa opakowujaca plik autorstwa krzyhaa
#include "cbitstream.h"
using std::string;
using std::cerr; 
// Class CVLAKFile
// klasa odpowiedzialna za operacje na naszym wlasnym formacie plikow
class CVLAKFile 
{
private:
	CFile file;
	bool afterHeader;
	CVLAKHeader cHeader;
public:
	// konstruktor
	CVLAKFile(const string& fileName, char *mode) : file(fileName.c_str(),mode), afterHeader(false)
	{}
	CVLAKHeader ReadHeader()
	{
		if(afterHeader) return cHeader; // taki trick
		char formatID[5]; formatID[4] = 0;
//		int samplingFreq = 0;
//		char bitsPerSample = 0, numOfChannels = 0, uid = 0;
		file.ReadNext(formatID,4);
		assert(string(formatID) == "VLAK");
			
		// deserializacja naglowka:
/* 
		file.ReadNext(&bitsPerSample,sizeof(char));
		file.ReadNext(&samplingFreq,sizeof(int));
		file.ReadNext(&numOfChannels,sizeof(char));
		file.ReadNext(&uid,sizeof(char));
*/		
		CBitStream bs(CVLAKHEADER_SIZE); // rozmiar naglowka ...
		file.ReadNext(bs.GetBytes(),CVLAKHEADER_SIZE); 
		cHeader = CVLAKHeader(bs);
		afterHeader = true;
		return cHeader;
	}
		
	// przez wskaznik
	// jesli EOF albo inny blad to zwracamy NULL
	// poniewaz pamiec jest przydzielana dynamicznie
	// to nie nalezy zapomniec o zwolnieniu jej!
	ICompressedFrame* ReadNextFrame()
	{
		unsigned short frameSize = 0;
		// assert(afterHeader)
		try // TODO: na razie szybka poprawka
		{
			file.ReadNext(&frameSize,sizeof(unsigned short));
			CBitStream stream(frameSize);
			file.ReadNext(stream.GetBytes(),frameSize); // odczy strumienia
			return ICompressedFrame::Deserialize(stream,cHeader);
		}
		catch(CFileException& cf) { return NULL; } // blad
	}
		
	void WriteHeader(const CVLAKHeader& header) // naglowek ma 4+11 = 15 bajtow
	{
		char formatID[] = "VLAK";
		file.Append(formatID,4);
		// a teraz robimy serializacje naglowka
/* 
		char tmp = header.GetBitsPerSample();
		file.Append(&tmp,sizeof(char));
		int int_tmp = header.GetSamplingFrequency();
		file.Append(&int_tmp,sizeof(int));
		tmp = header.GetNumOfChannels();
		file.Append(&tmp,sizeof(char));
		tmp = header.GetChannelManipulatorUID();
		file.Append(&tmp,sizeof(char));
*/
		CBitStream bs = header.Serialize();
		file.Append(bs.GetBytes(),bs.Size());
		afterHeader = true;
	}
	
	void WriteFrame(const ICompressedFrame& frame) // naglowek ramki 1+2 = 3 bajty (uid+size)
	{
		// TODO: trzeba okreslic jakie metody ma CBitStream
		// tym razem to proste:
		assert(afterHeader);
		CBitStream data = frame.Serialize();
		unsigned short dataSize = data.Size()+sizeof(unsigned char); // ilosc danych w strumieniu 
		file.Append(&dataSize,sizeof(unsigned short));
		// UWAGA!! teraz guid jest zapisywany tutaj
		unsigned char char_guid = (unsigned char)frame.GetGUID();
#ifdef DEBUG_GUID		
		std::cout<<"saving GUID= "<<(short)char_guid<<std::endl;
#endif		
		file.Append(&char_guid,sizeof(unsigned char));
		file.Append(data.GetBytes(),data.Size());
	}

	bool Eof() { return file.Eof(); }
};
#endif //CVLAKFILE_H

