// $Id: iencoder.h,v 1.10 2005/11/26 19:10:58 pmalinow Exp $
#ifndef IENCODER_H
#define IENCODER_H
#include "iostream"
#include "cvlakfile.h"
#include "cwavfile.h"
#include "ichannelmanipulator.h"
#include "ipredictor.h"
#include "ientropycompressor.h"
#include "assert.h" // albo cassert

// Interface IEncoder
// klasa abstrakcyjna odpowiedzialna za proces kodowania
// wav -> vlak
// np. klasa CLPCRiceEncoder : IEncoder koduje predyktorem LPC i Rice'em
class IEncoder 
{
public:
	// moga byc chronione
	IWavFileReader *inputFile;
	CVLAKFile *outputFile;

	virtual ~IEncoder() {};
	virtual void Encode() = 0;
};

// przykladowa implementacja
// mozna tez walnac klase szablonowa zamiast takiego gigantycznego konstruktora
class CEncoder : public IEncoder
{
protected:
	IChannelManipulator* manip;
	IPredictor* predictor;
	int blockSize;	
public:	
	// konstruktor
	CEncoder(IWavFileReader *input,CVLAKFile *output,int blockSize,
			IChannelManipulator* channelManipulator, IPredictor *predictor)
		: manip(channelManipulator),predictor(predictor), blockSize(blockSize)
	{
		inputFile = input;
		outputFile = output;
	}
	
	virtual void Encode()
	{
		IEntropyCompressor *compressor = NULL;
		IBlocksProvider* iBlocks = inputFile->GetBlocksProvider(blockSize);

		CBlock block = iBlocks->GetNextBlock();

		// zapisujemy naglowek
		outputFile->WriteHeader( CVLAKHeader(block.GetInfo() /*, manip->GetUID()*/));
		// teraz informacja o manipulatorze jest odczytywana bezposrednio
		// z bloku przez predyktor i zapisywana w GUIDzie ramki

		while(1) // TODO: zrobic to porzadnie
		{
			IFrame *frame = NULL;
			ICompressedFrame *cframe = NULL;
			// manipulacja blokiem
			block = manip->Manipulate(block);
			// modelowanie / predykcja
			frame = predictor->EncodeBlock(block); // new
			compressor = frame->GetCompressor(); // new
			// kompresja
#ifdef DEBUG_ENCODER
			std::cerr << *frame;
#endif
			cframe = compressor->CompressFrame(*frame); // new
#ifdef DEBUG_ENCODER
			std::cerr << "po kompresji:"<<*frame;
#endif
			// zapis ramki do pliku .vlak
			outputFile->WriteFrame(*cframe);
			// zwalniamy pamiec
			if(frame == cframe) // moze tak byc
			{
				delete frame; // frame = cframe = NULL;
			}
			else { delete frame; delete cframe; }
			delete compressor;
			
			if(!iBlocks->HasNext()) break;
			block = iBlocks->GetNextBlock();
		}
		
	}

	virtual ~CEncoder()
	{
		/*
		delete inputFile; delete outputFile;
		delete manip; delete predictor;
		*/
	}
};

// a tak sie wykorzystuje dziedziczenie ;)
class CSimpleEncoder : public CEncoder
{
	private:
		CSimpleChannelManipulator simpleManip;
		CSimplePredictor simplePred;
	public:
	CSimpleEncoder(IWavFileReader *input,CVLAKFile *output,int blockSize)
		: CEncoder(input,output,blockSize,&simpleManip,&simplePred)
	{}
	
};
#endif //IENCODER_H

