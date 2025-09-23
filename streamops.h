// $Id: streamops.h,v 1.6 2005/12/28 19:29:20 pmalinow Exp $
// Tutaj nalezy dodawac wlasne wersje operatora <<
#ifndef STREAMOPS_H
#define STREAMOPS_H
#include <ostream>
#include <iomanip>
#include <string>
#include <sstream> // ostringstream
#include "iinfo.h"
#include "cvlakheader.h"
#include "iframe.h"
#include "RiceCoder/cbitstream.h"
#include "uid.h"

// IInfo
std::ostream& operator<<(std::ostream& ostr, const IInfo& info);

// IFrame
std::ostream& operator<<(std::ostream& ostr, const IFrame& frame);

// CBitStream
std::ostream& operator<<(std::ostream& ostr, const CBitStream& bitStream);

// GUID
std::ostream& operator<<(std::ostream& ostr, const GUID& guid);

// PRZYDATNE PROCEDURKI

// konwersja typu na napis
// dla liczby: base - oznacza w jakim systemie ja wypisac
// width: szerokosc napisu (przydaje sie do formatowania
// UWAGA: to samo mozna zrobic korzystajac z iomanip
template<typename T> std::string toString(const T& obj,int base,int width,char fill = '0')
{
	std::ostringstream ostr;
	ostr<<std::setbase(base)<<std::setw(width)<<std::setfill(fill)<<obj;
	return ostr.str();
} // to musi byc tutaj :(

// specjalne wersje dla byte - wypisujemy jako liczby
std::string toString(const byte& obj,int base,int width, char fill = '0');


template<typename T> std::ostream& PrintArray(std::ostream& out, T* array,int size,
		int base, int width, const std::string& separator =" ", int columns = 5, char fill = '0')
{
	for(int i = 0; i< size; ++i)
	{
		out << toString(array[i],base,width,fill) << separator;
		if( (i+1)%columns == 0) out << std::endl;
	}
	return out;
}

char * predictorName(char uid);
char * frameName(char uid);
char * manipName(char uid);
char * coderName(char uid);

#endif

