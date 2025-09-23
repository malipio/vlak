// $Id: BlocksProviderFromWaveReader.cpp,v 1.5 2005/11/15 15:10:54 wwasiak Exp $
#include "BlocksProviderFromWaveReader.h"
#include "WaveFile.h"

BlocksProviderFromWaveReader::BlocksProviderFromWaveReader(CWaveReader * pMyNewWaveReader)
{ //nie ma konstruktora bezargumentowego - provider musi znac swojego readera
    pMyWaveReader=pMyNewWaveReader;
    iBlockSize=1024; //TODO defaultowo zeby cos bylo
}

CBlock BlocksProviderFromWaveReader::GetNextBlock()
{
    int iActBlockSize=iBlockSize; //moze sie zmniejszyc jesli jest koniec pliku
    if (pMyWaveReader->SamplesLeft() < iBlockSize)
	    iActBlockSize=pMyWaveReader->SamplesLeft(); //ten bedzie krotszy

    //to stworzenie cblocka ale rowniez przepisanie formatu z MyNewWaveReadera
    CBlock result(iActBlockSize,pMyWaveReader->BitsPerSample(),pMyWaveReader->SamplingRate(),pMyWaveReader->ChannelsCount());

    if (iActBlockSize<=0) return result; //mala ochrona na wypadek niedodatniego rozmiaru blocka

    CSample * tmpspace = new CSample[iActBlockSize * pMyWaveReader->ChannelsCount()];
    
    pMyWaveReader->GetSamples((void *)tmpspace, iActBlockSize);
    
    //przepisujemy naprzemiennie wystepujace sample kanalow w nieprzerwane ciagi kanalow jeden po drugim
    for (unsigned char ch=0; ch < pMyWaveReader->ChannelsCount(); ch++)
	{
	    for (int i=0; i<iActBlockSize; i++)
		{
//		    result.Data[i + iActBlockSize*ch] = tmpspace[i*pMyWaveReader->ChannelsCount() + ch]; //to samo co linijka wyzej - nie dziala przyjazn tu nie wiem dlaczego
		    result.GetSamples(0)[i + iActBlockSize*ch] = tmpspace[i*pMyWaveReader->ChannelsCount() + ch];
		}
	}
    
    delete [] tmpspace;
    
    return result;
}

bool BlocksProviderFromWaveReader::HasNext() const
{
    if (pMyWaveReader->SamplesLeft() > 0)
	    return true;
	else
	    return false;
}
