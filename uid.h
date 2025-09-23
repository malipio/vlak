// $Id: uid.h,v 1.4 2005/12/22 10:27:47 kjamroz Exp $
// Obsluga przechowywania informacji w UIDzie
#ifndef UID_H
#define UID_H
#include <assert.h>
class GUID 
{
private:
	unsigned char uid;
public:
	// frameUID - 3 bity (czyli mamy 8 mozliwych wartosci)
	// manipulatorUid - 3 bity 
	// coderUid - 2 bity (nie musi byc uzywany)
	GUID(unsigned char frameUID,
			unsigned char manipulatorUID, 
			unsigned char coderUID = 0)
	: uid(0)
	{
		assert( frameUID < 8);
		assert( manipulatorUID < 8);
		assert( coderUID == 0);
		uid = (coderUID<<6)|(frameUID<<3)|manipulatorUID;
	}
	
	// konstruktor deserializujacy
	GUID(unsigned char uid) : uid(uid) {} 
	
	// serializacja
	inline operator unsigned char(void) const
	{
		return (unsigned char)uid;
	}
	
	/*
	inline operator char(void)
	{
		return uid;
	}
	*/

	inline char GetManipulatorUID() const 
	{
		return uid&0x07;
	}

	inline char GetFrameUID() const
	{
		return (uid>>3)&0x07;
	}

	inline char GetCoderUID() const
	{
		return (uid>>6)&0x03;
	}
	
	// TODO: czy set'ery sa potrzebne?
};

// uid'y ramek
#define CSIMPLECOMPRESSEDFRAME_UID 	0
#define CLPCRICEFRAME_UID 		1
#define CFIRCOMPRESSEDFRAME_UID		2
#define CWAVELETCOMPRESSEDFRAME_UID	3
#define CFFTCOMPRESSEDFRAME_UID 	4

// uid'y manipulatorow
#define NOMANIPULATOR_UID		0
#define CSIMPLECHANNELMANIPULATOR_UID	0
#define CMIDSIDECHANNELMANIPULATOR_UID	1
#define CADAPTIVECHANNELMANIPULATOR_UID	2

// uid ricecodera (podawany domyslnie)
#define CRICECODER_UID			0
#define CSIMPLECOMPRESSOR		1

#endif
