// $Id: csample.h,v 1.3 2005/12/02 00:32:48 wwasiak Exp $
#ifndef CSAMPLE_H
#define CSAMPLE_H

#include <limits.h>

// TODO: poniewaz zdarza sie ze kodujemy sygnal roznicowy
// (np. MidSideChannelManipulator czy czesc residualna w predykcji)
// to wynik takich operacji moze przekroczyc zakres probki
// np. (zakres: -128:127, a wyniki 127-(-128) = 256)
// w takiej sytuacji konieczne jest utworzenie typu
// rozszerzonego - CExtSample. Sprawa otwarta pozostaje
// sposob zapisu takiej rozszerzonej probki...
// => mozemy np. serializowac unsigned shorty
// a dodatkowo dodac tablice 1 bitowych wartosci
// oznaczajacych znak liczby...

#define CSAMPLE_MAX SHRT_MAX
#define CSAMPLE_MIN SHRT_MIN

typedef signed short CSample;
/*
class CSample 
{
public:
	  CSample (char bitsPerSample=16);
};
*/

typedef signed int CExtSample; // rozszerzony rozmiar probki
#endif //CSAMPLE_H

