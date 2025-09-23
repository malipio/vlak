#include <iostream>
#include <fstream>
using std::cout;
using std::endl;
using std::ofstream;

#include <math.h>

#include "BitVector.h"
#include "WaveFile.h"
#include "fft.h"

#include "ientropycompressor.h"
#include "iencoder.h"

#include "cfftcompressedframe.h"
#include "cfftpredictor.h"
#include "RiceCoder/cricecoder.h"

void DoSthWithSamples(CWaveReader& aSource)
{
	sshort buf[20];
	aSource.GetSamples(buf,10);
	cout << "Samples:" << endl;
	cout << "#\tleft\tright" << endl;
	for(int i=0;i<10;++i)
	{
		cout << i << '\t' << buf[2*i] << '\t' << buf[2*i+1] << endl;
	}
}

void TestWriter()
{
	CWaveWriter wwriter;
	wwriter.Open("testwwriter.wav");

	CBlock block(1024), block2(123);
	wwriter.WriteHeader(block.GetInfo());

	for(int i=0;i<block.GetBlockSize();++i)
	{
		block.SetSample(0,i,(CSample)(3000*sin((10.0+i)/10.0)));
		block.SetSample(1,i,(CSample)(3000*sin(i/10.0)));
	}
	for(int i=0;i<block2.GetBlockSize();++i)
	{
		block2.SetSample(0,i,(CSample)(3000*sin(i/20.0)));
		block2.SetSample(1,i,(CSample)(3000*sin((i+10.0)/20.0)));
	}
	for(int j=0;j<100;++j)
	{
		wwriter.WriteBlock(block);
		wwriter.WriteBlock(block2);
	}

	wwriter.Close();
}

class CFFTCompressor : public IEntropyCompressor
{
public:
	CFFTCompressor(int blockSize);
	virtual ICompressedFrame* CompressFrame(IFrame& frame);
	virtual IFrame* DecompressFrame(ICompressedFrame& frame);

private:
	CFFT iFFT;
};

CFFTCompressor::CFFTCompressor(int blockSize) :
iFFT(blockSize)
{
}

ICompressedFrame* CFFTCompressor::CompressFrame(IFrame& frame)
{
	ICompressedFrame *tmp = dynamic_cast<ICompressedFrame *>(&frame);
	// nie robimy zadnej kompresji
	tmp->SetCompressedDataStream(tmp->GetDataStream());
	return tmp;
}
	
IFrame* CFFTCompressor::DecompressFrame(ICompressedFrame& frame)
{
	// TODO: konstruktor kopiujacy dla klas IFrame/ICompressedFrame !!!
	ICompressedFrame *tmp = &frame;
	tmp->SetDataStream( tmp->GetCompressedDataStream() );
	return tmp;
}

class CFFTEncoder : public CEncoder
{
	private:
		CSimpleChannelManipulator simpleManip;
		CSimplePredictor simplePred;
		CFFTCompressor fftComp;
	public:
	CFFTEncoder(IWavFileReader *input,CVLAKFile *output,int blockSize)
		: CEncoder(input,output,blockSize,&simpleManip,&simplePred),
		fftComp(blockSize)
	{}
};

#define sqr(a) (a)*(a)
void TestEncoder(const char* aName)
{
	int blockSize = 1024;
	
	CWaveReader wreader;
	CWaveWriter wwriter;

	wreader.Open(aName);
	wwriter.Open("encoded.wav");

	IBlocksProvider* blocks = wreader.GetBlocksProvider(blockSize);
	int blockcnt = 0;

	CFFTPredictor predictor;
	//predictor.SetParameters(1,1);

	while(blocks->HasNext() && blockcnt < 5000)
	{
		CBlock block = blocks->GetNextBlock();
		++blockcnt;

		if( blockcnt == 1 )
			wwriter.WriteHeader(block.GetInfo());

		if( block.GetBlockSize() != blockSize )
		{
			cout << "Ignored last block" << endl;
			break;
		}

		/*CSample* leftSamples = block.GetSamples(0);
		CSample* rightSamples = block.GetSamples(1);

		CFFT fft(blockSize);

		for(int i=0;i<0*blockSize;++i)
		{
			rightSamples[i] = 5123*sin(2*3.141592*i/100.0 +1) ;
			leftSamples[i] = rightSamples[i];
		}

		for(int tone=0;tone<10;tone++)
		{
			fft.FFT(rightSamples);
			//fft.CombineResults();

			ofstream fftlog("fft.log");
			for(int i=0;i<blockSize;++i)
				fftlog<<i<<"\t"<<fft.ResultsA()[i]<<"  \t"<<fft.ResultsB()[i]<<"  \t"<<sqrt(sqr(fft.ResultsA()[i])+sqr(fft.ResultsB()[i])) << endl;
			
			int maxFreq = 1;
			float maxAmpl = 0;
			
			// from 1 because 0 is constant component (freq=0) - it is stored separately
			// /2 because the result is symmetric (see signal theory).
			for(int i=1;i<blockSize/2;++i)
			{
				float a = fft.ResultsA()[i];
				float b = fft.ResultsB()[i];
				if( sqrt(a*a+b*b) >= maxAmpl )
				{
					maxAmpl = sqrt(a*a+b*b);
					maxFreq = i;
				}
			}

			float maxA = fft.ResultsA()[maxFreq];
			float maxB = fft.ResultsB()[maxFreq];

			int wndwidth = 1;
			for(int i=1;i<wndwidth;++i)
			{
				maxA += fft.ResultsA()[maxFreq-i];
				maxA += fft.ResultsA()[maxFreq+i];
				maxB = fft.ResultsB()[maxFreq-i];
				maxB = fft.ResultsB()[maxFreq+i];
			}

			//float maxPhase = fft.ResultsB()[maxFreq];
			float freq = 2*3.141592*maxFreq/blockSize;
			float constcomponent = 0.5*fft.ResultsA()[0];

			for(int i=0 ; i<blockSize ; ++i)
			{
				//rightSamples[i] -= maxAmpl * sin(maxPhase + i * 2*3.141592*blockSize/maxFreq);
				rightSamples[i] -= maxA*cos(i*freq) + maxB*sin(i*freq) + constcomponent;
				//leftSamples[i] = maxA*cos(i*freq) + maxB*sin(i*freq);
			}
		}*/

		//for(int i=0;i<blockSize;++i)
		//	block.SetSample(1,i,rightSamples[i]);

		CFFTCompressedFrame* frame =  dynamic_cast<CFFTCompressedFrame*>(predictor.EncodeBlock(block));

		CRiceCoder coder;
		coder.CompressFrame(*frame);

		coder.DecompressFrame(*frame);

		//frame->GetDataStream();
		//memcpy(block.GetSamples(0), frame->GetSamples(0), blockSize*2);
		//memcpy(block.GetSamples(1), frame->GetSamples(1), blockSize*2);

		//CBitStream stream = frame->GetDataStream();
		//frame->SetDataStream(stream);

		CBlock block2 = predictor.DecodeFrame(*frame);

		wwriter.WriteBlock(block);

		delete frame;
	}
	
	wwriter.Close();
}

int main(int argc,char** argv)
{
	//if( argc > 1 )
	//{
	//	try
	//	{
	//		CWaveReader wreader;
	//		wreader.Open(argv[1]);
	//		DoSthWithSamples(wreader);
	//		wreader.Close();
	//	}
	//	catch(CWaveFileFormatException& error)
	//	{
	//		cout << "Wave file format exception " << error.iErrorCode << ": " << error.Text() << endl;
	//	}
	//	catch(CFileException& fex)
	//	{
	//		cout << "File exception: errno=" << fex.iErrno << endl;
	//	}
	//}

	try
	{
		TestWriter();
	}
	catch(CFileException& fex)
	{
		cout << "File exception during TestWriter: errno=" << fex.iErrno << endl;
	}
	

	if( argc > 1 )
	{
		TestEncoder(argv[1]);
	}
	else
	{
		cout << "Usage: test file" << endl;
	}

	//CBitVector bv;
	//CFile bvfile("bv.bin","wb");
	//bv.Append(0x123,9);
	//bv.Append(0xf,3);
	//bv.Append(0x5,4);
	//bv.Append(1,1);
	//bv.End();
	//bv.AppendToFile(bvfile);
	return 0;
}
