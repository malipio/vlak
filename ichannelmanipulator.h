// $Id: ichannelmanipulator.h,v 1.8 2005/12/02 00:32:48 wwasiak Exp $
#ifndef ICHANNELMANIPULATOR_H
#define ICHANNELMANIPULATOR_H
#include "cblock.h"
#include "uid.h"
#include <iostream> // NULL

// Interface IChannelManipulator
// interfejs odpowiedzialny za manipulacje w bloku
// glownym celem jego powstania jest umozliwienie
// prostego definiowania operacji typu konwersja z LR na mid-side
class IChannelManipulator
{
public:
	virtual CBlock Manipulate(const CBlock& block) = 0;
		
	virtual CBlock Restore(const CBlock& block) = 0;
		
	// Teoria UIDow:
	// generalnie uidy sluza do rozpoznawania typu klasy
	// ktora ma zostac utworzona
	// Kazda klasa implementujaca ten interfejs powinna
	// przeslonic metode GetUID unikalna wartoscia.
	// Do odtworzenia obiektu sluzy metoda CreateInstance
	// ktora jednak bedzie prymitywna (chyba ze ktos chce sie popisac)
	virtual char GetUID() const = 0;
	
	virtual ~IChannelManipulator() {};
	
	static IChannelManipulator* CreateInstance(char uid);
};

class CSimpleChannelManipulator : public IChannelManipulator
{
	virtual CBlock Manipulate(const CBlock& block)
	{
		CBlock tmp = block;
		tmp.SetManipulatorUID(CSIMPLECHANNELMANIPULATOR_UID);
		return tmp;
	}
		
	virtual CBlock Restore(const CBlock& block)
	{
		CBlock tmp = block;
		tmp.SetManipulatorUID(NOMANIPULATOR_UID);
		return tmp;
	}
	
	virtual char GetUID() const { return CSIMPLECHANNELMANIPULATOR_UID; }
	
};

class CMidSideChannelManipulator : public IChannelManipulator
{
public:
	virtual CBlock Manipulate(const CBlock& block);
	virtual CBlock Restore(const CBlock& block);
	virtual char GetUID() const { return CMIDSIDECHANNELMANIPULATOR_UID; }
};

//ten manipulator bedzie tworzony tylko przy encodowaniu - sam wybierze najlepszy manipulator z w/w i uzyje jego
class CAdaptiveChannelManipulator : public IChannelManipulator
{
public:
	virtual CBlock Manipulate(const CBlock& block);
	virtual CBlock Restore(const CBlock& block); //nie bedzie uzywane w praktyce
	virtual char GetUID() const { return CADAPTIVECHANNELMANIPULATOR_UID; }
};

#endif //ICHANNELMANIPULATOR_H
