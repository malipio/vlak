// $Id: icompressedframe.cpp,v 1.8 2005/12/30 21:03:55 kjamroz Exp $
#ifndef ICOMPRESSEDFRAME_CPP
#define ICOMPRESSEDFRAME_CPP
#include "icompressedframe.h"
#include "csimplecompressedframe.h"
#include "clpcriceframe.h"
#include "cfircompressedframe.h"
#include "cwaveletcompressedframe.h"
#include "cfftcompressedframe.h"

// metody statyczne dla klas implemntujacych interfejs ICompressedFrame

ICompressedFrame* ICompressedFrame::CreateInstance(const GUID guid,const IInfo& info,CBitStream& stream)
{
	CSimpleCompressedFrame * ptr = NULL;
	
	switch(guid.GetFrameUID())
	{
		case CSIMPLECOMPRESSEDFRAME_UID : 
			ptr = new CSimpleCompressedFrame(info,0,
					guid.GetManipulatorUID()); // blockSize == 0 - nieznany
			 ptr->SetCompressedDataStream(stream);
			 return ptr;

		case CLPCRICEFRAME_UID : return new CLPCRiceFrame(info,guid,stream);
		case CFIRCOMPRESSEDFRAME_UID : return new CFIRCompressedFrame(info,guid,stream);
		case CWAVELETCOMPRESSEDFRAME_UID : return new CWaveletCompressedFrame(info,guid,stream);
		case CFFTCOMPRESSEDFRAME_UID : return new CFFTCompressedFrame(info,guid,stream);
		default : return NULL;
	}
	return NULL;
}

ICompressedFrame* ICompressedFrame::Deserialize (CBitStream& stream, const IInfo& info)
{ 
	unsigned char _guid;
	stream.GetNextByte(&_guid); // odczytujemy bajtowa reprezentacje GUIDa
	GUID guid(_guid);
//	unsigned char uid = guid.GetFrameUID();
#ifdef DEBUG_GUID	
	std::cout << "_guid = "<<(short)_guid<<std::endl;
	std::cout << "GUID = "<<guid<<std::endl;
#endif	
	// TODO: dopoki nie powstanie konstruktor deserializujacy dla tej ramki
	// to musi to pozostac w ten sposob
//	if(uid == CFIRCOMPRESSEDFRAME_UID) // UID == 2 ozn. CFIRCompressedFrame
//	{
	//TODO powinno to byc jakos przeniesione dp cfircompressedframe
//		CFIRCompressedFrame *ptr = dynamic_cast<CFIRCompressedFrame *>(CreateInstance(guid,info,stream));
		//teraz czytamy 6 nieskompresowanych csampli
//		stream.GetNextNBytes((byte *)ptr->startsamples,6*sizeof(CSample));
//		ptr->SetCompressedDataStream(stream);
//		return ptr;
//	}
//	else
	{
		// to sie powinno zawsze udac gdyz zawsze zapisujemy ramki
		// typu ICompressedFrame
		ICompressedFrame *ptr = CreateInstance(guid,info,stream);
		// teraz to wywolanie nie jest potrzebne, bo
		// robi to za nas konstruktor!
		//ptr->SetCompressedDataStream(stream);
		// tak stworzona ramka nie jest jeszcze uzywalna
		// trzeba ja bedzie zdekompresowac
		return ptr;
	}	
}

#endif //ICOMPRESSEDFRAME_CPP

