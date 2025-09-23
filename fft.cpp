#include <math.h>
#include <assert.h>
#include <memory.h>

#include "fft.h"

#ifndef NULL
#define NULL 0
#endif

CFFT::CFFT(uint aNumSamples) : iNumSamples(0),
iBuffer(NULL), iResultsA(NULL), iResultsB(NULL)
{
	SetNumSamples(aNumSamples);
}

CFFT::CFFT() : iNumSamples(0),
iBuffer(NULL), iResultsA(NULL), iResultsB(NULL)
{
}

CFFT::~CFFT()
{
	Free();
}

void CFFT::Free()
{
	delete [] iBuffer; iBuffer = NULL;
	delete [] iResultsA; iResultsA = NULL;
	delete [] iResultsB; iResultsB = NULL;
}

void CFFT::SetNumSamples(uint aNumSamples)
{
	if( iNumSamples == aNumSamples )
		return;  //nothing to do

	Free();

	iNumSamples = aNumSamples;
	iResultsA = new float[iNumSamples];
	iResultsB = new float[iNumSamples];
}

/** Calculates Fourier transform (not FFT!)
	in - input samples, destroyed
	a,b - results (sine and cosine coefficients)
	nin - number of input samples
	no - number of interesting output samples
	nzero - number of fake 0 samples appended at the end of the buffer
*/
void ft(float *in,float *a,float *b,int nin,int no,int nzero)
{
	int k,m;
	double s1,s2,ninrcp=1.0/(nin+nzero);
	s1=0;
	for(k=0;k<nin;k++)
	{
		s1+=in[k];
	}
	a[0]=(float)(2*s1*ninrcp);
	b[0]=0;
	for(m=1;m<no-1;m++)
	{
		s1=0;s2=0;
		for(k=0;k<nin;k++)
		{
			s1+=in[k]*cos((double)2*k*m*M_PI*ninrcp);
			s2+=in[k]*sin((double)2*k*m*M_PI*ninrcp);
		}
		a[m]=(float)(2*s1*ninrcp);
		b[m]=(float)(2*s2*ninrcp);
	}
	s1=0;
	for(k=0;k<nin;k++)
	{
		s1+=in[k]*cos((double)k*M_PI);
	}
	a[no-1]=(float)(2*s1*ninrcp);
	b[no-1]=0;
}

/*
   This computes an in-place complex-to-complex FFT
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform
*/
short fft(short int dir,long m,float *x,float *y)
{
   long n,i,i1,j,k,i2,l,l1,l2;
   double c1,c2,tx,ty,t1,t2,u1,u2,z;

   /* Calculate the number of points */
   n = 1;
   for (i=0;i<m;i++)
      n *= 2;

   /* Do the bit reversal */
   i2 = n >> 1;
   j = 0;
   for (i=0;i<n-1;i++) {
      if (i < j) {
         tx = x[i];
         ty = y[i];
         x[i] = x[j];
         y[i] = y[j];
         x[j] = tx;
         y[j] = ty;
      }
      k = i2;
      while (k <= j) {
         j -= k;
         k >>= 1;
      }
      j += k;
   }

   /* Compute the FFT */
   c1 = -1.0;
   c2 = 0.0;
   l2 = 1;
   for (l=0;l<m;l++) {
      l1 = l2;
      l2 <<= 1;
      u1 = 1.0;
      u2 = 0.0;
      for (j=0;j<l1;j++) {
         for (i=j;i<n;i+=l2) {
            i1 = i + l1;
            t1 = u1 * x[i1] - u2 * y[i1];
            t2 = u1 * y[i1] + u2 * x[i1];
            x[i1] = x[i] - t1;
            y[i1] = y[i] - t2;
            x[i] += t1;
            y[i] += t2;
         }
         z =  u1 * c1 - u2 * c2;
         u2 = u1 * c2 + u2 * c1;
         u1 = z;
      }
      c2 = sqrt((1.0 - c1) / 2.0);
      if (dir == 1)
         c2 = -c2;
      c1 = sqrt((1.0 + c1) / 2.0);
   }

   /* Scaling for forward transform */
   if (dir == 1) {
      for (i=0;i<n;i++) {
         x[i] /= n;
         y[i] /= n;
      }
   }

   return(true);
} 

void CFFT::FFT(float* buf)
{
	//ft(buf,iResultsA,iResultsB,iNumSamples,iNumSamples,0);
	memcpy(iResultsA, buf, iNumSamples * sizeof(float));
	memset(iResultsB, 0, iNumSamples * sizeof(float));

	int m = intlog2(iNumSamples);
	fft(1,m,iResultsA, iResultsB);

}

void CFFT::FFT(const sshort* buf, int bufLen)
{
	if( !iBuffer )
		iBuffer = new float[iNumSamples];

	//convert samples, fill rest with zeros
	for(uint i=0 ; i < iNumSamples ; ++i )
		iBuffer[i] = (float)( (i < (uint)bufLen) ? buf[i] : 0 );

	//do fft
	FFT(iBuffer);
}

void CFFT::FFT(const sshort* buf)
{
	if( !iBuffer )
		iBuffer = new float[iNumSamples];

	//convert samples
	for(uint i=0 ; i < iNumSamples ; ++i )
		iBuffer[i] = buf[i];

	//do fft
	FFT(iBuffer);
}

void CFFT::CombineResults()
{
	for(uint i=0 ; i < iNumSamples ; ++i )
	{
		float a = iResultsA[i];
		float b = iResultsB[i];

		//amplitude
		iResultsA[i] = sqrt(a*a+b*b); // * 2 / iNumSamples;
		
		//phase
		if( a!= 0 )
			iResultsB[i] = atan2(a,b);
		else
			iResultsB[i] = 0;
	}

}

int CFFT::FindLoudestFrequencyIndex() const
{
	double maxAmpl = 0;
	int maxFreq = 1;

	// starts from 1 because 0 is constant component (freq=0)
	// /2 because the result is symmetric.

	for(uint i=1;i<iNumSamples/2;++i)
	{
		float a = iResultsA[i];
		float b = iResultsB[i];
		float ampl = sqrt(a*a+b*b);
		if( ampl >= maxAmpl )
		{
			maxAmpl = ampl;
			maxFreq = i;
		}
	}

	return maxFreq;
}

void CFFT::RemoveFrequencyIndex(int aIndex)
{
	assert(aIndex >= 0 && aIndex < iNumSamples);

	iResultsA[aIndex] = 0;
	iResultsB[aIndex] = 0;
}

float CFFT::ConstantComponent() const
{
	return 0.5f * iResultsA[0];
}
