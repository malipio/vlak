// $Id: iframe.h,v 1.9 2005/11/26 19:10:58 pmalinow Exp $
#ifndef IFRAME_H
#define IFRAME_H
#include <ostream>
#include "cbitstream.h"
#include "iinfo.h"
#include "uid.h"

class IEntropyCompressor; // zeby nie trzeba bylo includowac

class IPredictor;
// Interface IFrame
// interfejs podstawowy dla ramek (ramka = skompresowany blok)
class IFrame
{
public:
	virtual ~IFrame() {};

	// zwraca informacje o przechowywanym strumieniu
	virtual const IInfo& GetInfo() const = 0;

	virtual int GetSize() const = 0;
	
	// zwraca fragment ramki ktory ma zostac poddany kompresji
	// w postaci strumienia
	virtual CBitStream GetDataStream() const = 0;
		
	// zwraca GUID, czyli uid zawierajacy informacje
	// o ramce, manipulatorze, coderze
	virtual const GUID GetGUID() const = 0;
	// zwraca UID ramki
	virtual char GetUID() const = 0; // TODO: zmienic nazwe na GetFrameUID
		
	// metoda ta bierze jako argument strumien danych
	// i ustawia swoje odpowiednie pola na jego podstawie
	// UWAGA: nie ma to nic wspolnego z Serialize/Deserialize!!
	virtual void SetDataStream(const CBitStream& stream) = 0;
		
	// metoda zwraca wskaznik na obiekt predyktora uzytego w tej ramce
	virtual IPredictor* GetPredictor() const = 0;

	// metoda wypisujaca informacje o klasie
	virtual std::ostream& Print(std::ostream& ostr) const = 0;
	
	// metoda zwraca wskaznik na obiekt ktory sluzy do kompresji tej ramke
	virtual IEntropyCompressor* GetCompressor() const = 0;
};
#endif //IFRAME_H

