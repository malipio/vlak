#ifndef FFT_H
#define FFT_H

#include <string.h>
#include "WaveTypes.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
#define PI M_PI

class CFFT
{
public:
	CFFT(uint aNumSamples);
	CFFT();
	~CFFT();

	void SetNumSamples(uint aNumSamples);

	/** Changes buf */
	void FFT(float* buf);

	void FFT(const sshort* buf);
	void FFT(const sshort* buf, int bufLen);

	template <class T>
	void iFFT(T* result);

	template <class T>
	void iFFT(const float *re, const float* im,T* result);

	/// Returns results (real or amplitude part) of last transformation
	const float* ResultsA() const
	{
        return iResultsA;
	}
	/// Returns results (imaginary or phase part) of last transformation
	const float* ResultsB() const
	{
        return iResultsB;
	}

	/// Returns results (real or amplitude part) of last transformation
	float* ResultsA()
	{
        return iResultsA;
	}
	/// Returns results (imaginary or phase part) of last transformation
	float* ResultsB()
	{
        return iResultsB;
	}

	/// Finds frequency with maximum amplitude
	int FindLoudestFrequencyIndex() const;

	void RemoveFrequencyIndex(int aIndex);

	float ConstantComponent() const;

	void CombineResults();

private:
	void Free();
	uint iNumSamples;

	/// Temporary buffer for conversion
	float* iBuffer;
	float* iResultsA;
	float* iResultsB;

};

static int intlog2(int val)
{
	int m=-1;
	for(int n=val ; n>0 ; n >>= 1)
		++m;
	return m;
}

short fft(short int dir,long m,float *x,float *y);

template <class T>
void CFFT::iFFT(T* result)
{
	int m = intlog2(iNumSamples);
	fft(-1, m, iResultsA, iResultsB);

	for(uint i=0 ; i < iNumSamples ; ++i )
		result[i] = (T)iResultsA[i];
    
}

template <class T>
void CFFT::iFFT(const float *re, const float* im,T* result)
{
	memcpy(iResultsA, re, iNumSamples * sizeof(float));
	memcpy(iResultsB, im, iNumSamples * sizeof(float));
	iFFT(result);
}


#endif //FFT_H
