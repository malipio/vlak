#include "cfftcompressedframe.h"
#include "cfftpredictor.h"

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int quantize(float value, float QFACTOR)
{
	int sign = (value>=0 ? 1 : -1);
	//
	int div = (int)((sign*value + QFACTOR/2) / QFACTOR);
	return sign*div;
}

int PowerUp(int arg) //zwraca najmniejsza potege dwojki nie mniejsza niz dana liczba
{
	const int powerz[] = {128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536};
	const int powerzsize = sizeof(powerz) / sizeof(int);
	int x;
	for (x=0; x<powerzsize; x++)
		if (powerz[x]>=arg) return powerz[x];
	return 0; //TODO error
};

//#define DEBUG_FFT

CBitStream* CFFTPredictor::CompressQuantizedSpectrum(int blockSize, int* re, int* im)
{
	vector<int> rlecounts;
	vector<int> reval, imval;

	int ssize = blockSize/2+1;
	
	//TODO: compress DC here?

	for(int cur=0;cur<ssize; )
	{
		if( re[cur] == 0 && im[cur] == 0 )
		{
			//sequence of zeros
			int zcount=0;
			while(cur < ssize && re[cur] == 0 && im[cur] == 0)
			{
				++zcount;
				++cur;
			}

			rlecounts.push_back(zcount-1);  //positive or 0 - sequence of zeros, dont store value - it's always 0
		}
		else
		{
			//nonzero elements
			int nzcount=0;
			while(cur < ssize && (re[cur] != 0 || im[cur] != 0) )
			{
				reval.push_back(re[cur]);
				imval.push_back(im[cur]);
				++nzcount;
				++cur;
			}
			rlecounts.push_back(-nzcount);
		}
	}

	//length in dwords
	size_t dwlen = 1 + rlecounts.size() + 2*reval.size();
	CBitStream* result = new CBitStream(dwlen*4);
	
#ifdef DEBUG_FFT
	cout << "dwlen*4" << dwlen*4 << endl;
#endif

	int tmp;
	tmp = (int)rlecounts.size();
	result->Write(&tmp,4);

	for(size_t i=0;i<rlecounts.size();++i)
	{
		tmp = rlecounts[i];
		result->Write(&tmp,4);
	}

	for(size_t i=0;i<reval.size();++i)
	{
		tmp = reval[i];
		result->Write(&tmp,4);
		tmp = imval[i];
		result->Write(&tmp,4);
	}

	return result;
}

void CFFTPredictor::DecompressQuantizedSpectrum(CBitStream* stream, int blockSize, int* re, int* im)
{
#ifdef DEBUG_FFT
	cout <<"Decompressing spectrum, streamsize="<<stream->Size()<<endl;
#endif

	vector<int> rlecounts;

	//read the data from stream
	int tmp;
	int nrlecounts;
	stream->GetNextNBytes((byte*)&nrlecounts,4);
	int ssize = blockSize/2 + 1;
	
#ifdef DEBUG_FFT
	cout <<nrlecounts << " " << ssize << endl;
#endif

	int curposition=0;

	for(int i=0;i<nrlecounts;++i)
	{
		stream->GetNextNBytes((byte*)&tmp,4);
		rlecounts.push_back(tmp);
	}

	for(int i=0;i<nrlecounts;++i)
	{
		assert(curposition<ssize);
		int cur = rlecounts[i];
		if( cur >= 0 )
		{
			//cur+1 times 0
			for(int j=0;j<cur+1;++j)
			{
				re[curposition]=0;
				im[curposition]=0;
				++curposition;
			}
		}
		else
		{
			//-cur nonzero values from stream
			for(int j=0;j<(-cur);++j)
			{
				stream->GetNextNBytes((byte*)&re[curposition],4);
				stream->GetNextNBytes((byte*)&im[curposition],4);
				++curposition;
			}			
		}
	}

#ifdef DEBUG_FFT
	cout <<"Decompressed spectrum: curposition="<<curposition<<endl;
#endif
}



IFrame* CFFTPredictor::EncodeBlock(const CBlock& block) //robi new - > potem trzeba to deletowac
{
	CFFTCompressedFrame * result=new CFFTCompressedFrame(block);

	// store predictor parameters
	result->QFACTOR = QFACTOR;
	result->MINRELAMPL = MINRELAMPL;

	int numSamples = block.GetBlockSize();
	int adjNumSamples = PowerUp(numSamples);
	int spectrumSize = adjNumSamples/2+1;

	iFFT.SetNumSamples(adjNumSamples);

	//predicted signal
	CExtSample* pred = new CExtSample[adjNumSamples];
	CExtSample* residual = new CExtSample[adjNumSamples];

	//quantized spectrum values, only half - because it is symmetric
	int* qre = new int[spectrumSize];
	int* qim = new int[spectrumSize];

	//const int maxfidx = numSamples/2 - 5; //iParam1;  //TODO: automatically detect best number of freqs

	for(int channel = 0 ; channel < block.GetInfo().GetNumOfChannels() ; ++channel )
	{
#ifdef DEBUG_FFT
		cout << "FFT for channel " << channel << " params: " << iParam1 << ", " << iParam2 << endl;
#endif
	    
		CFFTChannelFrame& chFrame = result->ChannelFrame(channel);

		//FFT:
		CSample* samples = block.GetSamples(channel);
		iFFT.FFT(samples, adjNumSamples);

		// Modify spectrum to be better compressible
		float *re = iFFT.ResultsA();
		float *im = iFFT.ResultsB();
	
#ifdef DEBUG_FFT
		cout << "Constant signal: " << re[0] << endl;
#endif

		//Find max amplitude
		int maxFreq = iFFT.FindLoudestFrequencyIndex();
		float sineAmpl = re[maxFreq];
		float cosineAmpl = im[maxFreq];
		float maxAmpl2 = sineAmpl*sineAmpl + cosineAmpl*cosineAmpl;
		//cout << "Maxfreq(idx): " << maxFreq << "\tsine=" << sineAmpl << "\tcosine=" << cosineAmpl << "\tmaxampl2=" << maxAmpl2 << endl;

		float minAbsAmpl2 = maxAmpl2 * MINRELAMPL2;

		//TODO: to quantize DC or not?
		int removedCnt=0;
		for(int fidx = 0 ; fidx < spectrumSize ; fidx ++ )
		{
			float a = re[fidx];
			float b = im[fidx];

			float ampl2 = a*a+b*b; 
#ifdef DEBUG_FFT
			cout << fidx << "\t" << a << "\t" << b;
#endif

			if( ampl2 > minAbsAmpl2 )
			{
				qre[fidx] = quantize(a,QFACTOR);
				qim[fidx] = quantize(b,QFACTOR);
			}
			else
			{
				++removedCnt;
				qre[fidx] = 0;
				qim[fidx] = 0;
			}

			re[fidx] = qre[fidx] * QFACTOR;
			im[fidx] = qim[fidx] * QFACTOR;

#ifdef DEBUG_FFT
			cout << "\t" << re[fidx] << "\t" << im[fidx] 
				//<< "\t" << re[numSamples - fidx] << "\t" << im[numSamples - fidx]
				<< endl;
#endif
			if( fidx > 0 && fidx != spectrumSize-1 )
			{
				//the spectrum is symmetric, but make it really symmetric
				//(complex numbers are z and z*)
				re[adjNumSamples - fidx] = re[fidx];
				im[adjNumSamples - fidx] = -im[fidx];
			}
		}

#ifdef DEBUG_FFT
		cout << "removed: " << removedCnt << endl;
#endif

		CBitStream* spectrum = CompressQuantizedSpectrum(adjNumSamples, qre, qim);
		chFrame.SetSpectrum(spectrum);  //takes ownership

		iFFT.iFFT(pred);

		// calculate residual signal, only for samples of block (not for added zeros)
		for(int sampleIdx = 0 ; sampleIdx < numSamples ; ++sampleIdx )
		{
			residual[sampleIdx] = samples[sampleIdx] - pred[sampleIdx];
		}

		// store residual signal
		chFrame.SetSamples(residual, numSamples);
	}

	delete residual;
	delete pred;

	return result;
}

CBlock CFFTPredictor::DecodeFrame(const IFrame& frame)
{
	const CFFTCompressedFrame& csFrame = 
		dynamic_cast<const CFFTCompressedFrame&>(frame);
		
#ifdef DEBUG_FFT
	cout << "DecodeFrame: blocksize=" << csFrame.GetBlockSize() << endl;
#endif

	//get params used to compression
	SetParameters(csFrame.QFACTOR, csFrame.MINRELAMPL);

	CBlock block(csFrame.GetInfo(),csFrame.GetBlockSize());
	block.SetManipulatorUID(csFrame.GetGUID().GetManipulatorUID());

	int nChannels = frame.GetInfo().GetNumOfChannels();

	int numSamples = csFrame.GetBlockSize();
	int adjNumSamples = PowerUp(numSamples);
	int spectrumSize = adjNumSamples/2+1;

	//quatized spectrum, half of range, temporary
	int* qre = new int[spectrumSize];
	int* qim = new int[spectrumSize];

	//spectrum
	float* re[2];// = new float*[nChannels];
	float* im[2];// = new float*[nChannels];

	int* residual = new int[adjNumSamples];
	int* predicted = new int[adjNumSamples];

	iFFT.SetNumSamples(adjNumSamples);

	//first decode spectrum data
	for(int channel = 0 ; channel < nChannels ; ++channel )
	{
		memset(qre,0,spectrumSize*4);
		memset(qim,0,spectrumSize*4);
		DecompressQuantizedSpectrum(&csFrame.DecompressedDataStream(),adjNumSamples,
			qre, qim);

		re[channel] = new float[adjNumSamples];
		im[channel] = new float[adjNumSamples];

		for(int i=0;i<spectrumSize;++i)
		{
			re[channel][i] = (float)qre[i] * QFACTOR;
			im[channel][i] = (float)qim[i] * QFACTOR;

			if(i>0 && i != spectrumSize-1)
			{
				re[channel][adjNumSamples-i] = re[channel][i];
				im[channel][adjNumSamples-i] = -im[channel][i];
			}
		}
	}
	
	for(int channel = 0 ; channel < nChannels ; ++channel )
	{
		CSample* samples = block.GetSamples(channel);

		//read residual signal
		csFrame.DecompressedDataStream().GetNextNBytes((byte*)residual, numSamples*sizeof(CExtSample));

		//predict
		iFFT.iFFT(re[channel], im[channel], predicted);

		//add predicted values and residual
		for(int sampleIdx = 0 ; sampleIdx < numSamples ; ++sampleIdx )
		{
			samples[sampleIdx] = predicted[sampleIdx] + residual[sampleIdx];
		}
	}

	delete [] qre;
	delete [] qim;
	for(int channel = 0 ; channel < nChannels ; ++channel )
	{
		delete [] re[channel];
		delete [] im[channel];
	}
	//delete [] re; delete [] im;
	delete [] predicted;
	delete [] residual;
	
	return block;
}

