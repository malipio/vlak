// $Id: idecoder.h,v 1.8 2005/12/28 19:30:59 pmalinow Exp $
#ifndef IDECODER_H
#define IDECODER_H
#include "cwavfile.h"
#include "cvlakfile.h"
#include "ichannelmanipulator.h"
#include "ientropycompressor.h"
#include "ipredictor.h"

//#define DEBUG_DECODER

// Interface IDecoder
// interfejs (albo klasa abstrakcyjna) do dekodowania
// wlasciwie to dekoder jest jeden bo jego dzialanie jest
// zawsze takie samo
class IDecoder
{
public:
	IWavFileWriter* wavFile; // plik wyjsciowy
	CVLAKFile* inputFile;
	
	virtual ~IDecoder() {}
	
	// ta metoda odwala cala czarna robote
	virtual void Decode() = 0;
};

class CDecoder : public IDecoder
{
public:
	CDecoder(CVLAKFile* input, IWavFileWriter* output)
	{
		inputFile = input;
		wavFile = output;		
	}
	
	virtual void Decode()
	{
		CVLAKHeader header = inputFile->ReadHeader();
//		IChannelManipulator* manip = NULL;
//			IChannelManipulator::
//			CreateInstance(header.GetChannelManipulatorUID());
		// zapisz naglowek
		wavFile->WriteHeader(header);
		
#ifdef DEBUG_DECODER
		cerr << header;
#endif
		while(1)
		{
			ICompressedFrame *cFrame = inputFile->ReadNextFrame(); // new
			if(inputFile->Eof()) break; // koniec pliku
#ifdef DEBUG_DECODER
			cerr << *cFrame;
#endif
			// teraz trzeba odtworzyc kompresor na podstawie ramki
			IEntropyCompressor *compressor = cFrame->GetCompressor(); // new
			
			IFrame * frame = compressor->DecompressFrame(*cFrame); // new
			// odtwarzamy predyktor
#ifdef DEBUG_DECODER
			cerr << *frame;
#endif
			IPredictor *predictor = frame->GetPredictor(); // new

			CBlock block = predictor->DecodeFrame(*frame);
			
			IChannelManipulator* manip = 
				IChannelManipulator::CreateInstance(block.GetManipulatorUID());
			// przywroc blok
			block = manip->Restore(block);
			// zapisz blok w pliku .wav
			wavFile->WriteBlock(block);
			
			// zwalniamy pamiec
			delete compressor; delete predictor; delete manip;
			if(cFrame == frame) { delete cFrame; }
			else { delete cFrame; delete frame; }
			// TODO: takie dzialanie jest kosztowne
			// lepiej zeby byl co najwyzej 1 obiekt danego typu 
			// predyktora i compressora --> singleton??
		}
		//delete manip;
	}

};

typedef CDecoder CSimpleDecoder; // proces dekodowania jest niezalezny od typu ramki
#endif //IDECODER_H

