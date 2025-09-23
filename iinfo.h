// $Id: iinfo.h,v 1.4 2005/11/15 20:23:56 pmalinow Exp $
#ifndef IINFO_H
#define IINFO_H
#include <ostream>
#include <iostream>
// Interface IInfo
// to tylko interfejs, konstruktora sami sobie napiszcie :P
class IInfo 
{
public:
	virtual ~IInfo() {};
	
	virtual char GetBitsPerSample() const = 0;
	
	virtual int GetSamplingFrequency() const = 0;
		
	virtual char GetNumOfChannels() const = 0;

	virtual std::ostream& Print(std::ostream& ostr) const
	{
		ostr << "bitsPerSample= "<<(int)GetBitsPerSample() << std::endl;
		ostr << "frequency= "<<GetSamplingFrequency() << std::endl;
		ostr << "numOfChannels= "<<(int)GetNumOfChannels() << std::endl;
		return ostr;

	}
};
#endif //IINFO_H

