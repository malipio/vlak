// $Id: cwaveletpredictor.h,v 1.2 2005/12/22 20:11:38 wwasiak Exp $
#ifndef IWAVELETPREDICTOR_H
#define IWAVELETPREDICTOR_H
#include "iframe.h"
#include "cblock.h"
#include "ipredictor.h"
//#include "csimplecompressedframe.h" // CSimpleCompressedFrame
#include "cwaveletcompressedframe.h"
#include <stdio.h>

//inkludy z waveleta
#include "invpacktree_int.h"
#include "packtree_int.h"
#include "support.h"
#include "delta.h"
#include "haar_int.h"
#include "ts_trans_int.h"
#include "line_int.h"
#include "costwidth.h"

class CWaveletPredictor : public IPredictor
{
public:
	IFrame* EncodeBlock(const CBlock& block) //robi new - > potem trzeba to deletowac
	{
		CWaveletCompressedFrame * result=new CWaveletCompressedFrame(block);
		//na razie spelnia kryteria iframe
		//teraz zaczynamy robic z niej compressedframe

		//tworzymy klase ktora zakompresuje nam waveletowo
		haar_int haar;
		ts_trans_int ts_trans;
		line_int<int *> line;
		liftbase<int *, int> &w = line;	//TODO mozna uzyc line albo haar albo ts_trans

		//liczymy najblizsza potege 2 w gore - to bedzie rozmiar wektora do data
		//niestety transformata falkowa przyjmuje ciagi o dl rownej potedze 2
		int realBlockSize=PowerUp(block.GetBlockSize());
		if (realBlockSize==0) RaiseException();
		
		//liczymy rozmiar nowego data
		int size = sizeof(int) * realBlockSize * block.GetInfo().GetNumOfChannels();
		CBitStream resultdata(size);

		//TODO przepisanie sampli do vectora intow
		int *vec = new int[ realBlockSize ];

		for (int i=0; i < block.GetInfo().GetNumOfChannels(); i++)
		    {
		    int s;
		    for (s=0; s < block.GetBlockSize(); s++) //wrzucamy caly jeden kanal do vec
			{
			vec[s]=block.GetSamples(0)[s+block.GetBlockSize()*i];
			}
		    for (s = block.GetBlockSize(); s < realBlockSize ; s++) //wypelniamy reszte 0ami
			{
			vec[s]=0;
			}
		    //teraz mamy juz caly vector (czyli caly 1 kanal) - poddajemy go transformacie
		    w.forwardTrans( vec, realBlockSize );
		    //i wypisujemy do resultdata
		    for (s=0; s < realBlockSize; s++)
			{
			resultdata.Write((byte *)(vec + s),sizeof(int));
			}
		    }

		//przepisujemy dane do data
		resultdata.Reset();
		result->SetDataStream(resultdata);
		delete[] vec;
		return result; //TODO juz z predykcja ale nie zakodowana ricem
	}
		
	virtual CBlock DecodeFrame(const IFrame& frame)
	{
		const CWaveletCompressedFrame& csFrame = 
			dynamic_cast<const CWaveletCompressedFrame&>(frame);
		CBlock block(csFrame.GetInfo(),csFrame.GetBlockSize());
		block.SetManipulatorUID(csFrame.GetGUID().GetManipulatorUID());

		//tworzymy klase ktora zakompresuje nam waveletowo
		haar_int haar;
		ts_trans_int ts_trans;
		line_int<int *> line;
		liftbase<int *, int> &w = line;	//TODO mozna uzyc line albo haar albo ts_trans

		//liczymy najblizsza potege 2 w gore - to bedzie rozmiar wektora do data
		//niestety transformata falkowa przyjmuje ciagi o dl rownej potedze 2
		int realBlockSize=PowerUp(block.GetBlockSize());
		if (realBlockSize==0) RaiseException();

		int *vec = new int[ realBlockSize ];
		CBitStream newdata(*csFrame.data,true); //TODO cala kopia - muzse robic bo tamto jest const
		newdata.Reset(); //zeby czytaj od poczatku tego cbitstreama

		for(int ch = 0; ch < frame.GetInfo().GetNumOfChannels(); ++ch)
		{
		    int s;
		    //wrzucamy caly jeden kanal do vec
		    for (s=0; s < realBlockSize; s++)
			{
			newdata.GetNextNBytes((byte *)(vec+s), sizeof(int));
			}
		    //transformata odwrotna
		    w.inverseTrans( vec, realBlockSize );
		    //zrzut do tablicy samples
		    for (s=0; s < block.GetBlockSize(); s++)
			{
			block.GetSamples(0)[s+block.GetBlockSize()*ch]=(CSample)vec[s];
			}
		}
		delete[] vec;
		return block;
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


	void RaiseException() //TODO
	    {
	    printf("Fatal: Blocksize is too big!");
	    exit(1);
	    };
};


#endif //CWAVELETPREDICTOR_H
