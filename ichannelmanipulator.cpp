// $Id: ichannelmanipulator.cpp,v 1.5 2005/12/02 00:32:48 wwasiak Exp $
#include "ichannelmanipulator.h"


CBlock CMidSideChannelManipulator::Manipulate(const CBlock& block)
{
    CBlock result(block);
    result.SetManipulatorUID(GetUID());
    if (block.GetInfo().GetNumOfChannels()!=2) return result; //manipulator tylko dla sygnalow stereofonicznych
    for (int i=0; i<block.GetBlockSize(); i++)
	{
	    long l,r,m,s; //left, right, mid, side(=l-r)
	    l=block.GetSamples(0)[i];
	    r=block.GetSamples(1)[i];
	    m=l+r;
	    if (m % 2) m-=1; //zaokraglamy przy dzieleniu przez 2 zawsze w strone ujemnej nieskonczonosci :)
	    m/=2;
    	    s=l-r;
	    result.GetSamples(0)[i]=(CSample)m;
	    result.GetSamples(1)[i]=(CSample)s;
	}
    return result;
}


CBlock CMidSideChannelManipulator::Restore(const CBlock& block)
{
    CBlock result(block);
    result.SetManipulatorUID(NOMANIPULATOR_UID);
    if (block.GetInfo().GetNumOfChannels()!=2) return result; //manipulator tylko dla sygnalow stereofonicznych
    for (int i=0; i<block.GetBlockSize(); i++)
	{
	    long l,r,m,s,d; //left, right, mid, side(=l-r), delta (pomocnicza)
	    l=r=m=block.GetSamples(0)[i]; //najpierw zakladamy ze sa l i r sa rowne mid a potem liczymy delte - roznice
	    s=block.GetSamples(1)[i];
	    //delta dla lewego (jesli side nieparzyste, delty beda rozne)
	    d=s/2; //na obcinamy czesc po przecinku ilorazu (czyli zaokraglamy w strone 0)
	    if (s>0 && (s%2!=0)) d+=1; //jesli mid jest nieparzyste i dodatnie to delte zwiekszamy o 0
	    l+=d;
	    //delta dla prawego
	    d=-s/2; //na obcinamy czesc po przecinku ilorazu (czyli zaokraglamy w strone 0)
	    if (s<0 && (s%2!=0)) d+=1; //jesli mid jest nieparzyste i ujemne to delte zwiekszamy o 0
	    r+=d;
	    result.GetSamples(0)[i]=(CSample)l;
	    result.GetSamples(1)[i]=(CSample)r;
	}
    return result;
}


CBlock CAdaptiveChannelManipulator::Manipulate(const CBlock& block)
{
    
    if (block.GetInfo().GetNumOfChannels()!=2) //tylko stereo da sie obrabiac innymi manipulatorami niz simple
	{
	CBlock result(block);
	result.SetManipulatorUID(CSIMPLECHANNELMANIPULATOR_UID);
	return result; //nie ma potrzeby robienia do tego csimple'a
	}
    for (int i=0; i < block.GetBlockSize(); i++)
	{
	    int x=block.GetSamples(0)[i]; //musi bcy wykorzystany int, zeby wynik sie nie obcial do shorta
	    x-=block.GetSamples(1)[i];
	    if (x>CSAMPLE_MAX || x<CSAMPLE_MIN)
		{
		    CBlock result(block);
		    result.SetManipulatorUID(CSIMPLECHANNELMANIPULATOR_UID);
		    return result; //nie ma potrzeby robienia do tego csimple'a
		}
	}

    //jesli tu jestesmy - midside sie nadaje najlepiej
    CMidSideChannelManipulator manip;
    CBlock result=manip.Manipulate(block);
    result.SetManipulatorUID(CMIDSIDECHANNELMANIPULATOR_UID);
    return result;
}


//to z zalozenia nie bedzie wykonywane - ale byc musi pro forma
CBlock CAdaptiveChannelManipulator::Restore(const CBlock& block)
{
    return block;
}


// metody statyczne
IChannelManipulator* IChannelManipulator::CreateInstance(char uid)
	{
		// cala filozofia:
		if( uid == CSIMPLECHANNELMANIPULATOR_UID) return new CSimpleChannelManipulator;
		if( uid == CMIDSIDECHANNELMANIPULATOR_UID) return new CMidSideChannelManipulator;
		if( uid == CADAPTIVECHANNELMANIPULATOR_UID) return new CAdaptiveChannelManipulator; //TODO nie wiem czy potrzebne
		
		return NULL;
	}
