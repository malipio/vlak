// $Id: testlpc.cpp,v 1.8 2005/12/22 10:10:17 kjamroz Exp $
// Testy dla LPC
#include "iinfo.h"
#include "clpcriceframe.h"
#include "clpcpredictor.h"
#include "WaveFile.h" // WaveReader
#include "streamops.h" // przedefiniowane operatory<<
#include "uid.h"
#include <iostream>
#include <fstream>
#include <map>
#include <math.h>
using namespace std;

char * wavName = "x.wav"; // std. plik ktory wszyscy maja
// char * wavDecName = "x_dec.wav";
char * resName = "res.gnu";
int blockSize = 1024;
char minOrder = 1;
char maxOrder = 10;

void residualGnuPlot(ostream& os,CLPCRiceFrame& frame)
{
	// dla kanalu 0
	map<CExtSample,int> tab;
	double EX = 0.0;
	
	for(int i = 0; i< frame.GetBlockSize()-frame.order[0]; ++i)
		tab[frame.GetResiduals(0)[i]] = 0; // init
	
	for(int i = 0; i< frame.GetBlockSize()-frame.order[0]; ++i)
	{
		tab[frame.GetResiduals(0)[i]]++;
		EX += frame.GetResiduals(0)[i];
	}

	// TODO: jak te parametry sie powinno wyznaczac?
	// geometryczny vs. wykladniczy
	EX /= frame.GetBlockSize()-frame.order[0];
/*	
	double p = 0.0;
	double VX;
	p = 1.0/(EX+1.0);
	cerr << "p = "<<p <<endl;
	VX = (1.0-p)/(p*p); // VX = EX2 -(EX)^2 => EX2 = VX+(EX)^2
	cerr << "VX = "<<VX<<endl;
	double EX2 = VX+EX*EX;
	cerr << "EX2 = "<<EX2<<endl;
*/	
	//os <<"p = "<<p<<endl;
	os <<"plot '-' title 'f'";
	os <<", '-' title 'EX' with impulses";
	//os <<", p*(1.0-p)**abs(x)";
	os<<endl;
	
	for(int i = 0; i< frame.GetBlockSize()-frame.order[0]; ++i)
	{
		os << frame.GetResiduals(0)[i]<<
			" "<<tab[frame.GetResiduals(0)[i]]<<endl;
	}
	os << endl << "e"<<endl<<EX<<" 6"<<endl;
	os << endl << "e"<<endl;
	os<<"pause -1 'waiting....'"<<endl;
}

int debugLPC()
{
	cout<<"debugLPC()"<<endl;
	CWaveReader waveReader;
	waveReader.Open(wavName);
	IBlocksProvider* blocks = waveReader.GetBlocksProvider(blockSize);
	CBlock block = blocks->GetNextBlock();
	CLPCPredictor predictor;
	
	predictor.SetMinOrder(minOrder);
	predictor.SetMaxOrder(maxOrder);
	CLPCRiceFrame *frame = (CLPCRiceFrame *)predictor.EncodeBlock(block);
	assert(frame != NULL);
	ofstream file(resName);
	residualGnuPlot(file,*frame);
	cout << *frame << endl;
	delete frame;
	return 0;
}

int constLPC()
{
	cout<<"constLPC()"<<endl;
	CBlock block(1024,16,44100,2); // tworzymy pusty block
	block.SetManipulatorUID(NOMANIPULATOR_UID);
	for(int i = 0; i < block.GetBlockSize(); ++i)
	{
		block.SetSample(0,i,/*i*/0);
		block.SetSample(1,i,/*(CSample)(1000.0*sin(i))*/0);
	}
	
	//PrintArray(cout,block.GetSamples(0),blockSize,10,4);
	CLPCPredictor predictor;
	
	predictor.SetMinOrder(minOrder);
	predictor.SetMaxOrder(maxOrder);
	CLPCRiceFrame *frame = (CLPCRiceFrame *)predictor.EncodeBlock(block);
	assert(frame != NULL);
	IEntropyCompressor *comp = frame->GetCompressor();
	ICompressedFrame *frame2 = comp->CompressFrame(*frame);
	assert(frame2 != NULL);
	
	CBitStream bs = frame2->Serialize();
	cout << bs << endl;
	// deserializacja
	CLPCRiceFrame frame3(frame2->GetInfo(),frame->GetGUID(),bs); // konstruktor deserializujacy
	assert(frame->GetUID() == frame2->GetUID());
	cout << frame3<<endl;
	IEntropyCompressor *comp2 = frame3.GetCompressor();
		comp2->DecompressFrame(frame3);
	delete comp; delete comp2;
	CBlock block2 = predictor.DecodeFrame(frame3);

	for(int b = 0; b < block.GetBlockSize(); ++b)
		if((block.GetSamples(0)[b] != block2.GetSamples(0)[b])
				||
			(block.GetSamples(1)[b] != block2.GetSamples(1)[b])
		  ) { cout << "errr"<<endl; return 1;}
	cout << *frame << endl;
	delete frame;
	return 0;
}

int losslessLPC()
{
	cout << "losslessLPC()"<<endl;
	CWaveReader waveReader;
	waveReader.Open(wavName);
	IBlocksProvider* blocks = waveReader.GetBlocksProvider(blockSize);
	while(blocks->HasNext())
	{
		CBlock block = blocks->GetNextBlock();
		CLPCPredictor predictor;			
		predictor.SetMinOrder(minOrder);
		predictor.SetMaxOrder(maxOrder);
		CLPCRiceFrame *frame = (CLPCRiceFrame *)predictor.EncodeBlock(block);
		assert(frame != NULL);
		CBlock block2 = predictor.DecodeFrame(*frame);
		assert(block.GetBlockSize() == block2.GetBlockSize());
		for(char chNum = 0; chNum < block.GetInfo().GetNumOfChannels(); ++chNum)
			for(int i = 0; i < block.GetBlockSize(); ++i)
			{
				if(block.GetSamples(chNum)[i] != block2.GetSamples(chNum)[i])
				{
					cout << "["<<toString(i,10,4)<<
						"] org = "<<block.GetSamples(chNum)[i];
					cout << " dec = "<<block2.GetSamples(chNum)[i]<<endl;
				}
			}
			
		delete frame;
	}
	return 0;
	
}

int serialization()
{
	cout << "serialization()"<<endl;
	CWaveReader waveReader;
	waveReader.Open(wavName);
	IBlocksProvider* blocks = waveReader.GetBlocksProvider(blockSize);
	CLPCPredictor predictor;
	
	predictor.SetMinOrder(minOrder);
	predictor.SetMaxOrder(maxOrder);
	for(CBlock block = blocks->GetNextBlock(); blocks->HasNext(); 
			block = blocks->GetNextBlock())
	{
		CLPCRiceFrame *frame = (CLPCRiceFrame *)predictor.EncodeBlock(block);
		// testujemy czy sie serializuje :)
		IEntropyCompressor *comp = frame->GetCompressor();
		frame = dynamic_cast<CLPCRiceFrame *>(comp->CompressFrame(*frame)); // bez new ...
		//cout << "Przed serializacja:"<<endl;
		//cout << *frame <<endl;
		CBitStream bs = frame->Serialize();
		// deserializacja
		char uid;
		bs.GetNextByte((byte *)&uid);
		assert(uid == frame->GetUID());
		CLPCRiceFrame des(frame->GetInfo(),frame->GetGUID(),bs); // konstruktor deserializujacy
		IEntropyCompressor *comp2 = des.GetCompressor();
		comp2->DecompressFrame(des);
		//cout << "Po deserializacji:" <<endl;
		//cout << des << endl;
		assert(frame != NULL);
		delete comp; delete comp2;
		delete frame;
	}
	return 0;
}

int testUID()
{
	cout << "testUID()" << endl;
	GUID uid(CLPCRICEFRAME_UID,CMIDSIDECHANNELMANIPULATOR_UID);
	cout << uid<<endl;
	cout << "frameUID= "<<frameName(uid.GetFrameUID())<<endl;
	cout << "manipUID= "<<manipName(uid.GetManipulatorUID())<<endl;
	cout << "coderUID= "<<coderName(uid.GetCoderUID())<<endl;
	return 0;
}
int main(int argc, char* argv[])
{
	int retval = 0;	
	if(argc == 1) // pusta cmd
	{
		cout << argv[0] <<" [wavName] [blockSize]"<<endl;
		cout << "domyslnie: "<<wavName;
		cout << " blockSize= "<<blockSize<<endl;
	} else
	{
		if(argc > 1 ) wavName = argv[1]; 
		if(argc > 2 ) blockSize = atoi(argv[2]);
		if(blockSize == 0) { cout << "eRRRor" << endl; return 1; }
	}
/*	
	retval = debugLPC();
	cout << (retval==0?"OK.":"NOT OK!!")<<endl;
	retval = losslessLPC();
	cout << (retval==0?"OK.":"NOT OK!!")<<endl;

	retval = serialization();
	cout << (retval==0?"OK.":"NOT OK!!")<<endl;
*/	
	retval = constLPC();
	cout << (retval==0?"OK.":"NOT OK!!")<<endl;
	
	retval = testUID();
	cout << (retval==0?"OK.":"NOT OK!!")<<endl;
	return retval;
}
