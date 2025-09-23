// $Id: clpcriceframe.h,v 1.7 2005/11/26 19:10:58 pmalinow Exp $
#ifndef CLPCRICEFRAME_H
#define CLPCRICEFRAME_H
#include <assert.h>
#include <iostream>
#include "uid.h"
#include "icompressedframe.h"
#include "ientropycompressor.h"
#include "csample.h"
#include "iinfo.h"
#include "cbitstream.h"
#include "streamops.h"

// Ramka dla predyktora LPC (Linear Predictive Coding
// wspolczynniki predyktora NIE sa kwantyzowane
// (zapamietywane sa jako float'y)
// UID = 01
class CLPCRiceFrame : public ICompressedFrame 
{
private:
	const IInfo& info;
	CBitStream *compressed; // skompresowany sygnal
	int blockSize;
	GUID guid;
public:
	char *order; // chNum
	// order[chNum] == 0 oznacza ze ramka zawiera tylko stale
	// wartosci -> wyznacznik macierzy R jest rowny 0 ->
	// co oznacza ze nie mozna wyznaczyc wspolczynnikow LPC
	// w takiej sytuacji w startSamples[chNum][0] znajdzie
	// sie wartosc stalej 
	// TODO: nie jest to tak naprawde
	// potrzebne bo autoc[0] == 0 <=> gdy ramka jest stala
	// i ma wartosc 0
	
	// TODO: TBD faktycznie wychodzi na to ze fajnie by bylo
	// dzielic ramke na podramki - oddzielne dla kazdego
	// kanalu, moznaby wtedy stosowac dla jednego LPC a
	// dla drugiego Fixed (wielomianowy) albo Const
	
	float **coefficients; // order x chNum
	CExtSample **residualPart; // blockSize-order x chNum
	CSample **startSamples; // order x chNum

	inline int GetBlockSize() const { return blockSize; }

	inline CSample* GetStartSamples(char chNum) //const
	{
		return startSamples[chNum];
	}

	inline float * GetCoefficients(char chNum) //const
	{
		return coefficients[chNum];
	}

	inline CExtSample* GetResiduals(char chNum) //const
	{
		return residualPart[chNum];
	}
	
	inline void Init()
	{
		order = NULL;
		coefficients = NULL;
		residualPart = NULL;
		startSamples = NULL;
		compressed = NULL;
		
	}
	
	CLPCRiceFrame(const IInfo& iinfo, int blockSize, char manipUID);
	
	void InitArrays(char order[], bool initResidual = true); // pozostale arg. sa brane z IInfo
	
	// implementacja IFrame
	
	virtual ~CLPCRiceFrame();

	virtual const IInfo& GetInfo() const;

	virtual int GetSize() const;
	
	virtual CBitStream GetDataStream() const; // TODO TBD: trzeba dla kazdego kanalu osobno?
		
	virtual const GUID GetGUID() const { return guid; }
	virtual char GetUID() const { return guid.GetFrameUID(); }
		
	// metoda ta bierze jako argument strumien danych
	// i ustawia swoje odpowiednie pola na jego podstawie
	// UWAGA: nie ma to nic wspolnego z Serialize/Deserialize!!
	virtual void SetDataStream(const CBitStream& stream);
		
	// metoda zwraca wskaznik na obiekt predyktora uzytego w tej ramce
	virtual IPredictor* GetPredictor() const ;

	// metoda wypisujaca informacje o klasie
	virtual std::ostream& Print(std::ostream& ostr) const;

	// implementacja ICompressedFrame
	
	virtual void SetCompressedDataStream (const CBitStream& compressedDataStream);
	
	// pobiera skompresowany strumien w celu dekompresji
	virtual const CBitStream& GetCompressedDataStream() const;
	
	// serializuje obiekt do strumienia
	virtual CBitStream Serialize() const;
	
	// metoda zwraca wskaznik na obiekt ktory skompresowal ta ramke
	virtual IEntropyCompressor* GetCompressor() const;
	
	// konstruktor deserializujacy
	CLPCRiceFrame(const IInfo& info,const GUID guid, CBitStream& stream);
};
#endif //CLPCRICEFRAME_H

