// $Id: testincludes.cpp,v 1.12 2005/12/19 01:25:01 wwasiak Exp $

//#define DEBUG_GUID
#include "cbitstream.h"
#include "cblock.h"
#include "clpcriceframe.h"
#include "csample.h"
#include "cvlakfile.h"
#include "cvlakheader.h"
#include "cwavfile.h"
#include "iblocksprovider.h"
#include "ichannelmanipulator.h"
#include "icompressedframe.h"
#include "idecoder.h"
#include "iencoder.h"
#include "ientropycompressor.h"
#include "iframe.h"
#include "iinfo.h"
#include "ipredictor.h"
#include "WaveFile.h" // klasy krzyhaa
#include "streamops.h" // przedefiniowane operatory<<
#include "cfirpredictor.h"
#include "clpcpredictor.h"
#include "RiceCoder/cricecoder.h"
#include <iostream>
#include "cwaveletpredictor.h"
#include "cwaveletcompressedframe.h"

using namespace std;

char * wavName = "x.wav"; // std. plik ktory wszyscy maja
char * wavDecName = "x_dec.wav";
int blockSize = 1024;

int testCVLAKFile(CVLAKFile& infile)
{
	// testujemy klase CVLAKFile
	//CVLAKHeader header(16,44100,2,1);
	//{
	//	CVLAKFile file("tst.vlak","w");
	//	file.WriteHeader(header);
	//}

	{ 
		//CVLAKFile infile("tst.vlak","r");
		CVLAKHeader tmp = infile.ReadHeader();
		cout << tmp << endl;
//		cout << "bitsPerSample= "<<(int)tmp.GetBitsPerSample() << endl;
//		cout << "frequency= "<<tmp.GetSamplingFrequency() << endl;
//		cout << "numOfChannels= "<<(int)tmp.GetNumOfChannels() << endl;
//		cout << "manipulatorUID= "<<(int)tmp.GetChannelManipulatorUID() << endl;
	}

	return 0;
}

int testEncoderDecoder()
{
	{
		CVLAKFile vlak("enc_dec.vlak","wb");
		CWaveReader waveReader;
		waveReader.Open(wavName);
		CSimpleEncoder encoder(&waveReader,&vlak,blockSize);
		// wio!
		encoder.Encode();
		// pliki powinny sie zamknac automatycznie
	}
	
	{
		CVLAKFile vlak("enc_dec.vlak","rb");
		CWaveWriter waveWriter;
		waveWriter.Open(wavDecName);
		CSimpleDecoder decoder(&vlak, &waveWriter);
		
		decoder.Decode();
		waveWriter.Close(); // zamykamy plik - zeby zapisal sie naglowek
		testCVLAKFile(vlak);
	}
	
	return 0;
}

// to jest niby test dla MidSideChannelManipulatora
// tzn. czy nie ma segmentation fault ;)
int testMidSide()
{
	CSimplePredictor simplePred;
	CMidSideChannelManipulator manip; //!!
	{
		CVLAKFile vlak("midside.vlak","wb");
		CWaveReader waveReader;
		waveReader.Open(wavName);
		CEncoder encoder(&waveReader,&vlak,blockSize,&manip,&simplePred);
		encoder.Encode();
	}
	
	{
		CVLAKFile vlak("midside.vlak","rb");
		CWaveWriter waveWriter;
		waveWriter.Open(wavDecName);
		CSimpleDecoder decoder(&vlak, &waveWriter);
		
		decoder.Decode();
		waveWriter.Close(); // zamykamy plik - zeby zapisal sie naglowek
	}

	return 0;
}

int testFIREncoder() // lepiej nie laczyc z MidSide bo moze wybuchnac ;-)
{
	CFIRPredictor firPred;
	//CRiceCoder Comp;
	//CSimpleCompressor simpleComp;
	CAdaptiveChannelManipulator manip; //!!
	{
		CVLAKFile vlak("fir.vlak","wb");
		CWaveReader waveReader;
		waveReader.Open(wavName);
		CEncoder encoder(&waveReader,&vlak,blockSize,&manip,&firPred);
		encoder.Encode();
	}
	
	{
		CVLAKFile vlak("fir.vlak","rb");
		CWaveWriter waveWriter;
		waveWriter.Open(wavDecName);
		CSimpleDecoder decoder(&vlak, &waveWriter);
		
		decoder.Decode();
		waveWriter.Close(); // zamykamy plik - zeby zapisal sie naglowek
	}

	return 0;
}

int testWaveletEncoder() // lepiej nie laczyc z MidSide bo moze wybuchnac ;-)
{
	CWaveletPredictor Pred;
	CAdaptiveChannelManipulator manip;
	{
		CVLAKFile vlak("wavelet.vlak","wb");
		CWaveReader waveReader;
		waveReader.Open(wavName);
		CEncoder encoder(&waveReader,&vlak,blockSize,&manip,&Pred);
		encoder.Encode();
	}
	
	{
		CVLAKFile vlak("wavelet.vlak","rb");
		CWaveWriter waveWriter;
		waveWriter.Open(wavDecName);
		CSimpleDecoder decoder(&vlak, &waveWriter);
		
		decoder.Decode();
		waveWriter.Close(); // zamykamy plik - zeby zapisal sie naglowek
	}

	return 0;
}


int testLPCEncoder()
{
	CLPCPredictor lpcPred;
	CAdaptiveChannelManipulator manip;
	{
		CVLAKFile vlak("lpc.vlak","wb");
		CWaveReader waveReader;
		waveReader.Open(wavName);
		
		// ten predyktor ma opcje
		lpcPred.SetMinOrder(1);
		lpcPred.SetMaxOrder(32);
		//lpcPred.SetOrder(8); // nie adaptacyjnie
		
		CEncoder encoder(&waveReader,&vlak,blockSize,&manip,&lpcPred);
		encoder.Encode();
	}
	
	{
		CVLAKFile vlak("lpc.vlak","rb");
		CWaveWriter waveWriter;
		waveWriter.Open(wavDecName);
		CDecoder decoder(&vlak, &waveWriter);
		
		decoder.Decode();
		waveWriter.Close(); // zamykamy plik - zeby zapisal sie naglowek
	}

	return 0;
}

int main(int argc, char* argv[0])
{
	int retval = 0;	
	if(argc == 1) // pusta cmd
	{
		cout << argv[0] <<" [wavName] [wavDecName] [blockSize]"<<endl;
		cout << "domyslnie: "<<wavName<<" => "<<wavDecName;
		cout << " blockSize= "<<blockSize<<endl;
	} else
	{
		if(argc > 1 ) wavName = argv[1]; 
		if(argc > 2 ) wavDecName = argv[2];
		if(argc > 3 ) blockSize = atoi(argv[3]);
		if(blockSize == 0) { cout << "eRRRor" << endl; return 1; }
	}
	
//	retval = testEncoderDecoder();	
//	cout << (retval==0?"OK.":"NOT OK!!")<<endl;
//	retval = testMidSide();	
//	cout << (retval==0?"OK.":"NOT OK!!")<<endl;

	retval = testFIREncoder();
	cout << (retval==0?"OK.":"NOT OK!!")<<endl;
	retval = testLPCEncoder();
	cout << (retval==0?"OK.":"NOT OK!!")<<endl;
	retval = testWaveletEncoder();
	cout << (retval==0?"OK.":"NOT OK!!")<<endl;

	return retval;
}
