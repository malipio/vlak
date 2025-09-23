// $Id: clpcpredictor.cpp,v 1.6 2005/12/28 19:34:00 pmalinow Exp $
#include <iostream>
#include <math.h>
#include "clpcpredictor.h"

//#define LPC_DEBUG
using namespace std;

// funkcja oblicza autokorelacje sygnalu
// maxLag == rzad predyktora
// r[] - tablica w ktorej zapiszemy wyniki (musi byc typu double)
void CLPCPredictor::ComputeAutocorrelation(CSample samples[],int samplesCount,
		int maxLag, double r[])
{
	for(int l = 0; l < maxLag; ++l)
	{
		double sum = 0.0;
		for(int n = l; n < samplesCount; ++n)
			sum+=(double)samples[n]*(double)samples[n-l];
		// nie robimy normalizacji
		r[l] = sum;
	}
	// z czego wynika ze jesli r[0] == 0.0 to sygnal jest staly
}

void CLPCPredictor::ComputeResidual(CLPCRiceFrame& lpcFrame, const CBlock& block)
{
	for(int chNum = 0; chNum < lpcFrame.GetInfo().GetNumOfChannels(); ++chNum)
	{
		if(lpcFrame.order[chNum] == 0) continue; // no residual
		float *a = lpcFrame.GetCoefficients(chNum);
		CExtSample *res = lpcFrame.GetResiduals(chNum);
		for(int k = lpcFrame.order[chNum]; k < block.GetBlockSize(); ++k)
		{
			double s = 0.0;
			for(int i = 0; i < lpcFrame.order[chNum]; ++i)
				s += a[i]*block.GetSamples(chNum)[k-i-1]; 
			
			// UWAGA na bledy zaokraglenia
			res[k-lpcFrame.order[chNum]]= (CExtSample)(block.GetSamples(chNum)[k]-(CSample)s); // !!!
#ifdef LPC_DEBUG				
			cout << "predicted sample = "<<s<<endl;
			cout << "original sample  = "<<block.GetSamples(chNum)[k]<<endl;
			cout << "residual         = "<<res[k-lpcFrame.order[chNum]]<<endl;
#endif				
		}
	}
}

void CLPCPredictor::ReproduceSamples(CLPCRiceFrame& lpcFrame, CBlock& block)
{
	for(char chNum = 0; chNum < lpcFrame.GetInfo().GetNumOfChannels(); ++chNum)	
	{
		if(lpcFrame.order[chNum] == 0) // ramka z wartoscia stala
		{
			for(int i = 0; i < lpcFrame.GetBlockSize(); ++i)
				block.SetSample(chNum,i,lpcFrame.GetStartSamples(chNum)[0]);
			continue;
		}

		for(int i = 0; i < lpcFrame.order[chNum]; ++i)
			block.SetSample(chNum,i,lpcFrame.GetStartSamples(chNum)[i]);
		float * a = lpcFrame.GetCoefficients(chNum);
		CSample * s = block.GetSamples(chNum);
		for(int k = lpcFrame.order[chNum]; k < block.GetBlockSize(); ++k)
		{
			double sk = 0.0;
			for(int i = 0; i < lpcFrame.order[chNum]; ++i)
				sk += a[i]*s[k-i-1];
			// UWAGA na bledy zaokraglenia
			block.SetSample(chNum,k,(CSample)((CSample)sk + 
						lpcFrame.GetResiduals(chNum)[k-lpcFrame.order[chNum]]));
		}
	}
}

void CLPCPredictor::ComputeCoefficients(int chNum,double r[], float *c[], double error[])
{
	// TODO zoptymalizowac - czyli zeby dzialalo tak jak FLAC :P
	// Levinson-Durbin 
	// forma nieoptymalizowana
	// w tej wersji jest jednak >>numerycznie niestabilny<<
	double *a = new double[maxOrder+1]; // indeksujemy od 1
	//lpcFrame->GetCoefficients(chNum);
	double *lambda = new double[maxOrder]; // backward error
	double *delta = new double[maxOrder]; // forward error
	double *k = new double[maxOrder+1]; 
	double *ta = new double[maxOrder+1]; // tymczasowa kopia a
	// init:
	lambda[0] = r[0];
	delta[0] = r[1];
	k[1] = delta[0]/lambda[0];
	a[1] = k[1]; ta[1] = a[1];
	c[0][0] = a[1];
	// recursion:
	for(int p = 1; p < maxOrder; ++p) // ? p < maxOrder-1
	{
		//lambda[p] = lambda[p-1] - k[p]*delta[p-1];
		//rownowazne:
		lambda[p] = lambda[p-1]*(1 - k[p]*k[p]);

		// podobno lambda[p] to blad predykcji dla
		// modelu rzedu p
		// podobno wypada tez podzielic go
		// przez blockSize...
		double mul = 0.0;
		// delta[p]=r[p]-[Ap-1]*[ro*#p-1]
		for(int i = 0; i < p; ++i)
			mul += a[i+1]*r[p-i]; // ?
		delta[p] = r[p] - mul;
		k[p+1] = delta[p]/lambda[p]; // podejrzane
		
		// modyfikacja tablicy wsp. rzedu p
		// => jesli chcemy mozemy je zapamietac
		// w ten sposob mozemy przy okazji
		// wyznaczyc wszystkie rzedy od 1 do maxOrder

		for(int i = 1; i <= p; ++i)
			c[p][i-1] = a[i]; // zapamietujemy wsp. dla aktualnego rzedu
		
		// Ap+1 = [ Ap - k[p+1]*Ap# | k[p+1] ]
		for(int i = 1; i <= p; ++i)
			ta[i] -= k[p+1]*a[p-i+1]; // trzeba operowac na kopii
		
		for(int i = 1; i <= p; ++i)
			a[i] = ta[i]; // przywracamy wartosci poprawne

		a[p+1] = k[p+1]; ta[p+1] = a[p+1];
	}


	for(int i = 0; i < maxOrder; ++i)
		c[maxOrder-1][i]=a[i+1]; // kopiujemy wspolczynniki
	
//	cout << "delta {"<<endl;
//	PrintArray(cout,delta,maxOrder,10,0);
//	cout << "};"<<endl;
#ifdef LPC_DEBUG		
	cout << "k {"<<endl;
	PrintArray(cout,k+1,maxOrder,10,0);
	cout << "};"<<endl;
#endif
	memcpy(error,lambda,maxOrder*sizeof(double));
	delete[] lambda; delete[] delta; delete[] k;
	delete[] a; delete[] ta;

}

// libFLAC
void CLPCPredictor::FLAC__lpc_compute_lp_coefficients(const double autoc[], unsigned max_order, float *lp_coeff[] ,double error[])
{
	unsigned i, j;
	double r, err, *ref, *lpc;

	assert(max_order > 0);
	ref = new double[max_order];
	lpc = new double[max_order];

	err = autoc[0];

	for(i = 0; i < max_order; i++) {
		/* Sum up this iteration's reflection coefficient. */
		r = -autoc[i+1];
		for(j = 0; j < i; j++)
			r -= lpc[j] * autoc[i-j];
		ref[i] = (r/=err);

		/* Update LPC coefficients and total error. */
		lpc[i]=r;
		for(j = 0; j < (i>>1); j++) {
			double tmp = lpc[j];
			lpc[j] += r * lpc[i-1-j];
			lpc[i-1-j] += r * tmp;
		}
		if(i & 1)
			lpc[j] += lpc[j] * r;

		err *= (1.0 - r * r);

		/* save this order */
		for(j = 0; j <= i; j++)
			lp_coeff[i][j] = (float)(-lpc[j]); /* negate FIR filter coeff to get predictor coeff */
		error[i] = err;
	}
	
	delete[] ref;
	delete[] lpc;
}
	
void CLPCPredictor::SetMinOrder(char order)
{
	assert(order > 0);
	minOrder = order;
}

void CLPCPredictor::SetMaxOrder(char order)
{
	assert(order > 0);
	maxOrder = order;
}

// pomocnicza funkcyjka
void CLPCPredictor::SetOrder(int order)
{
	SetMinOrder(order);
	SetMaxOrder(order);
}

//#define DEBUG_BESTORDER
// zwraca najlepszy rzad predyktora, czyli taki dla ktorego ramka
// powinna miec najmniejszy rozmiar.
// JAK TO ZROBIC?: oszacowac parametr k i na jego podstawie oszacowac rozmiar
// sygnalu residualnego po kompresji -> wybieramy ramke o najmniejszym
// rozmiarze
int CLPCPredictor::SelectBestOrder(float *c[],double error[],char maxOrder, int blockSize)
{
	assert(minOrder <= maxOrder);
	assert(minOrder > 0);
	
	double VX ;
	double k;
	int *totalSize = new int[maxOrder];
	for(int i = 0; i< maxOrder; ++i)
	{
		VX = error[i]/(blockSize-i-1);
#ifdef DEBUG_BESTORDER		
		cout<<"error= "<<error[i]<<endl;
		cout<<i+1<<" VX= "<<VX<<endl;
#endif
		// szacujemy parametr k
		// korzystam z:
		// Tony Robinson - SHORTEN ... pdf
		//k = log(log(2)*sqrt(VX/2))/log(2); // LOL
		k = log(sqrt(VX/2.0))/log(2.0); // LOL
		k = k < 0 ? 0 : ceil(k); // dziwne ze moze byc k < 0 !!
#ifdef DEBUG_BESTORDER		
		cout<<i+1<<" k= "<<k<<endl;
#endif		
		// ok, czyli wiemy ze kazda probka bedzie miala co najmniej
		// k bitow przy odpowiedniej kompresji
		// a wiec mozemy oszacowac rozmiar sygnalu residualnego
		// przez ilosc_probek*k <lol>

		totalSize[i] = (int)(k+1)*(blockSize-i-1)/8 + 1; // w bajtach
		// dodajemy do tego stale rozmiary zalezne od rzedu predyktora
		totalSize[i] += (i+1)*(sizeof(float)+sizeof(CSample));
#ifdef DEBUG_BESTORDER		
		cout <<"estimatedSize = "<<totalSize[i]<<endl;
#endif
		// to jest tylko jakis tam szacunek niekoniecznie poprawny
		// - taki sposob jest jednak duzo szybszy niz porownywanie
		// rzeczywistych rozmiarow skompresowanych strumieni
	

		// TBD: inny patentem dla adaptacyjnego LPC jest sprawdzanie
		// czy blad w kolejnych rzedach zmniejszyl sie o epsilon
		// -> kontynuujemy az bledy predykcji sie ustabilizuja
		// z dokladnoscia do np. epsilon = 0.1
	}

	double min_size = totalSize[minOrder-1];
	int min_idx = minOrder-1;
	
	for(int i = min_idx+1; i < maxOrder; ++i)
		if(totalSize[i] < min_size)
		{
			min_size = totalSize[i];
			min_idx = i;
		}
	// mamy minimum
#ifdef DEBUG_BESTORDER		
	cout << "min order idx= "<<min_idx<<endl;
#endif
	delete[] totalSize;
	return min_idx+1; // zwracamy najlepszy rzad a nie indeks...
		
}

IFrame* CLPCPredictor::EncodeBlock(const CBlock& block)
{
	assert(minOrder <= maxOrder);
	assert(minOrder > 0); // 0 nie jest poprawna wartoscia
	//assert(minOrder == maxOrder); // adaptacyjnosc juz powinna dzialac
	
	CLPCRiceFrame *lpcFrame = new CLPCRiceFrame(block.GetInfo(),
			block.GetBlockSize(),
			block.GetManipulatorUID());
	
	// ustalenie rzadu dla kazdego kanalu z osobna
	char *orders = new char[block.GetInfo().GetNumOfChannels()];

// TODO: poprawic zeby oszczedzac pamiec	
	float **bestCoeffs = new float *[block.GetInfo().GetNumOfChannels()];

	for(char chNum = 0; chNum < block.GetInfo().GetNumOfChannels(); ++chNum)
	{
		double *r = new double[maxOrder+1];
		bestCoeffs[chNum] = NULL; // init
		
		// liczymy autokorelacje
		ComputeAutocorrelation(block.GetSamples(chNum),
				block.GetBlockSize(),maxOrder+1, r); // !!! tak wynika z algorytmu
		if(r[0] == 0.0) {
#ifdef LPC_DEBUG			
			cerr << "NO SIGNAL!"<<endl;
#endif			
			orders[chNum] = 0;
			delete[] r;
			continue;
		}
		
		double *error = new double[maxOrder]; // blad predykcji dla rzedow 1..order
		float **coeffs = new float *[maxOrder];
		for(int i= 0; i < maxOrder; ++i)
			coeffs[i] = new float[maxOrder]; // przydzial pamieci
#ifdef LPC_DEBUG			
		cout << "autocorrelation {"<<endl;
		PrintArray(cout,r,maxOrder,10,0," ",5);
		cout << "};"<<endl;
#endif			
			// liczymy wspolczynniki predyktora
#ifdef LPC_MY
		ComputeCoefficients(chNum,r,c,error); // maxOrder?
/*		
	#ifdef LPC_DEBUG			
		cout << "my coeffs {"<<endl;
		PrintArray(cout,lpcFrame->GetCoefficients(chNum),maxOrder,10,0);
		cout << "};"<<endl;
		
		cout << "my error {"<<endl;
		PrintArray(cout,error,maxOrder,10,0);
		cout << "};"<<endl;
	#endif
*/	
#else
		FLAC__lpc_compute_lp_coefficients(
				r, maxOrder, coeffs, error);

		char bestOrder = SelectBestOrder(coeffs,error,maxOrder,block.GetBlockSize());
		orders[chNum] = bestOrder;
		bestCoeffs[chNum] = coeffs[bestOrder-1];
	#ifdef LPC_DEBUG
		cout << "flac error {"<<endl;
		PrintArray(cout,error,maxOrder,10,0);
		cout << "};"<<endl;

		cout << "flac best order = "<<(short)bestOrder<<endl;
		
		cout << "flac best coeffs {"<<endl;
		PrintArray(cout,coeffs[bestOrder-1],bestOrder,10,0);
		cout << "};"<<endl;
	#endif
#endif
		delete[] error;
		delete[] r;

		for(int i= 0; i < maxOrder; ++i)
			if(i != bestOrder-1) delete[] coeffs[i];
		delete[] coeffs;
	}
	
	lpcFrame->InitArrays(orders,true);
	
	
	// kopiujemy probki startowe
	// oraz przepisujemy wartosci tymczasowe...
	for(int chNum = 0; chNum < block.GetInfo().GetNumOfChannels(); ++chNum)
	{
		if(orders[chNum] == 0) { 
			lpcFrame->GetStartSamples(chNum)[0] = block.GetSamples(chNum)[0];
			continue;
		}
		
		memcpy(lpcFrame->GetCoefficients(chNum),bestCoeffs[chNum],
				orders[chNum]*sizeof(float));
		for(int i = 0; i < orders[chNum]; ++i)
			lpcFrame->GetStartSamples(chNum)[i]=block.GetSamples(chNum)[i];
	}
	ComputeResidual(*lpcFrame,block);

	for(int chNum = 0; chNum < block.GetInfo().GetNumOfChannels(); ++chNum)
		if(bestCoeffs[chNum] != NULL) delete[] bestCoeffs[chNum];
	delete[] orders;
	delete[] bestCoeffs;
	return lpcFrame;
}
	
CBlock CLPCPredictor::DecodeFrame(const IFrame& frame)
{
	const CLPCRiceFrame *lpcFrame = dynamic_cast<const CLPCRiceFrame *>(&frame);
	CBlock block(lpcFrame->GetInfo(),lpcFrame->GetBlockSize());
	block.SetManipulatorUID(lpcFrame->GetGUID().GetManipulatorUID());
	
	ReproduceSamples(const_cast<CLPCRiceFrame&>(*lpcFrame),block);
	return block;
}

