// $Id: cvlakheader.h,v 1.5 2005/11/26 19:10:58 pmalinow Exp $
#ifndef CVLAKHEADER_H
#define CVLAKHEADER_H
#include "iinfo.h"
#include "cbitstream.h"


// Class CVLAKHeader
// naglowek pliku .vlak 
class CVLAKHeader : public IInfo 
{
private:
	int samplingFrequency;
	char bitsPerSample, numOfChannels; //, manipulatorUid;
public:
//	char GetChannelManipulatorUID() const
//	{
//		return manipulatorUid;
//	}
	
	// konstruktor
	// uid - identyfikator uzytego manipulatora kanalow
	CVLAKHeader(char bitsPerSample = 0, int freq = 0, char numOfChannels = 0 /*, char uid = 0*/)
		: samplingFrequency(freq), bitsPerSample(bitsPerSample),
			numOfChannels(numOfChannels) //, manipulatorUid(uid)
	{}
	
	CVLAKHeader(const IInfo& i/*,char uid*/)
		: samplingFrequency(i.GetSamplingFrequency()),
		  bitsPerSample(i.GetBitsPerSample()),
		  numOfChannels(i.GetNumOfChannels())
		  //,manipulatorUid(uid) 
		  {}
		

	// implementacja interfejsu IInfo
	virtual char GetBitsPerSample() const { return bitsPerSample; }
	
	virtual int GetSamplingFrequency() const { return samplingFrequency; }
		
	virtual char GetNumOfChannels() const { return numOfChannels; }
	
	virtual std::ostream& Print(std::ostream& ostr) const
	{
		IInfo::Print(ostr);
//		ostr << "manipulatorUID= "<<
//			(int)GetChannelManipulatorUID() << std::endl;
		return ostr;
	}

	// serializacja, deserializacja
	CBitStream Serialize() const
	{
#define CVLAKHEADER_SIZE 6		
		CBitStream bs(1+4+1); // = 6
		char tmp = GetBitsPerSample();
		bs.Write(&tmp,sizeof(char));
		int int_tmp = GetSamplingFrequency();
		bs.Write(&int_tmp,sizeof(int));
		tmp = GetNumOfChannels();
		bs.Write(&tmp,sizeof(char));

		return bs; // full copy
	}

	CVLAKHeader(CBitStream& stream)
	{
		stream.GetNextNBytes((byte *)&bitsPerSample,sizeof(char));
		stream.GetNextNBytes((byte *)&samplingFrequency,sizeof(int));
		stream.GetNextNBytes((byte *)&numOfChannels,sizeof(char));
	}
};
#endif //CVLAKHEADER_H

