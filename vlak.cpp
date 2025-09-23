// $Id: vlak.cpp,v 1.4 2006/01/12 02:09:04 kjamroz Exp $
// frontend do vlaka

#include <iostream>
#include <string>
#include <getopt.h>
#include <exception>
#include <string.h>

#include "streamops.h" // przedefiniowane operatory<<
#include "idecoder.h"
#include "iencoder.h"
#include "WaveFile.h"
#include "cfirpredictor.h"
#include "clpcpredictor.h"
#include "cwaveletpredictor.h"
#include "cfftpredictor.h"
#include "analyzer.h"

/*
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
#include "ientropycompressor.h"
#include "iframe.h"
#include "iinfo.h"
#include "ipredictor.h"
#include "ricecoder/cricecoder.h"
#include "cwaveletcompressedframe.h"
*/

using namespace std;

class BadOptionException : public exception 
{
	private:
		string reason;
	public:
		BadOptionException(string reason = "") throw()
		{
			this->reason = "BadOptionException: "+reason;
		}

	virtual ~BadOptionException() throw () {}
	virtual const char *what() const throw() {
		return reason.c_str();
	}
};

class VLAKFrontend {
	private:
		static const char * rev;
		const int argc;
		char **argv; // const
		enum AnalyzeFlag {
			UNSET = 0, RESIDUAL_DISTRIB = 1, ORIGINAL = 2, PREDICTED = 3, 
			ORDERS = 4, INFO = 5, MANIPULATED = 6, RESIDUAL = 7, CR = 8
		};
	protected:
		int blockSize;
		int predictor;
		int manipulator;
		int minOrder,maxOrder;
		
		float fft_qfactor, fft_minrelampl;
		
//		int encoder;
		char *inputFileName, *outputFileName;
		void PrintHelp(ostream& os);
		void PrintSettings(ostream& os)
		{
			os << "predictor type   : "<<predictorName(predictor)<<endl;
			os << "manipulator type : "<<manipName(manipulator)<<endl;
			os << "blocksize        : "<<blockSize<<endl;
			os << "lpcorder         : "<<minOrder<<","<<maxOrder<<endl;
			if(inputFileName != NULL)
				os << "input            : "<<inputFileName<<endl;
			if(outputFileName != NULL)
				os << "output           : "<<outputFileName<<endl;
		}

		void ReplaceExtension(string& fn, const string &in_ext, const string &out_ext)
		{
			// TODO:
			// rozwazyc: plik.wavek zostanie zamieniony na plik.vlak
			// rozwazyc: plik.wav.cosik -> plik.vlak
			// moze tak byc?
			string::size_type st = fn.rfind(in_ext);
			if(st == string::npos) { fn += out_ext; return; }
			string tmp = fn.substr(0,st);
			fn = tmp+out_ext; // albo fn.replace
			
		}
		void Encode()		
		{
			if(inputFileName == NULL)
				throw BadOptionException("no input file name!");
			string tmp(inputFileName);
			if(outputFileName == NULL) {
				ReplaceExtension(tmp,".wav",".vlak");
				outputFileName = const_cast<char *>(tmp.c_str());
//				cerr << "output= "<< outputFileName << endl;				
			}
			cerr << "settings:"<<endl;
			PrintSettings(cerr);
			IChannelManipulator * manip = IChannelManipulator::
					CreateInstance(manipulator);
			if(manip == NULL)
				throw BadOptionException("invalid manipulator type");
			IPredictor *pred = IPredictor::CreateInstance(predictor);
			if(pred == NULL)
				throw BadOptionException("invalid predictor type");
			IAdaptivePredictor *a_pred = 
				dynamic_cast<IAdaptivePredictor *>(pred);
			if(a_pred != NULL) 
			{
				a_pred->SetMinOrder(minOrder);
				a_pred->SetMaxOrder(maxOrder);
			}
			
			CFFTPredictor* fft_pred = dynamic_cast<CFFTPredictor*>(pred);
			if( fft_pred != NULL )
			{
				cerr << "FFT parameters   : Qfactor=" << fft_qfactor << ", MinRelAmpl=" << fft_minrelampl << endl;
				fft_pred->SetParameters(fft_qfactor, fft_minrelampl);			    
			}
			
			{
				CVLAKFile vlak(outputFileName,"wb");
				CWaveReader waveReader;
				waveReader.Open(inputFileName);
				
				// TODO: wypisywac informacje o postepie kodowania
				CEncoder encoder(&waveReader,&vlak,blockSize,manip,pred);
				encoder.Encode();
			}
			
			delete manip;
			delete pred;
			cerr << "OK."<<endl;
		}

		void PrintDecodeInfo(ostream& os) 
		{
			if(inputFileName != NULL)
				os << "input            : "<<inputFileName<<endl;
			if(outputFileName != NULL)
				os << "output           : "<<outputFileName<<endl;			
		}
			
		void Decode()
		{
			if(inputFileName == NULL)
				throw BadOptionException("no input file name!");
			string tmp(inputFileName);
			if(outputFileName == NULL) {
				ReplaceExtension(tmp,".vlak",".wav");
				outputFileName = const_cast<char *>(tmp.c_str());
			}
			PrintDecodeInfo(cerr);
			CVLAKFile vlak(inputFileName,"rb");
			CWaveWriter waveWriter;
			waveWriter.Open(outputFileName);
			CDecoder decoder(&vlak, &waveWriter);
			// TODO wypisywac jakies informacje o postepie dekodowania
			// decoder.AddFrameHandler ...
			decoder.Decode();
			waveWriter.Close(); // zamykamy plik - zeby zapisal sie naglowek
			cerr << "OK"<<endl;
		}

		void Analyze(AnalyzeFlag flag)
		{
#ifdef FRONTEND_DEBUG			
			cerr << "analyze_flag= "<< flag << endl;
#endif			
			if(inputFileName == NULL)
				throw BadOptionException("no input file name!");
			VLAKAnalyzer anal(inputFileName);
			switch(flag)
			{
				case RESIDUAL_DISTRIB : anal.ProduceResidualDistribution(); break;
				case ORIGINAL : anal.ProduceOriginalSignal(); break;
				case MANIPULATED : anal.ProduceManipulatedSignal(); break;
				case RESIDUAL : anal.ProduceResidualSignal(); break;
				case ORDERS : anal.ProducePredictorOrders(); break;
				case INFO : anal.ProduceInfo(); break;
				case CR : anal.ProduceCompressionRatio(); break;
				case PREDICTED: throw BadOptionException("not implemented yet");
				case UNSET: throw BadOptionException("no analyze option specified");
			}
		}

	public:
		VLAKFrontend(int argc, char *argv[]) : argc(argc), 
						       argv(argv)
		{
			// ustawienia domyslne
			blockSize = 1024;
			predictor = 1; // LPC
			manipulator = 2; // AdaptiveMidSide
			minOrder = 1; maxOrder = 12;
    			inputFileName = outputFileName = NULL;
			
			fft_qfactor = 8.0f; fft_minrelampl = 0.01f;
		}

		void Run();
};

const char * VLAKFrontend::rev = "$Revision: 1.4 $";

void VLAKFrontend::PrintHelp(ostream& os) 
{
	os << "VLAK - flac inspired audio codec " <<rev<<endl;
	os << "(c) Piotr Malinowski, Wojtek Wasiak, "<<
		"Krzysiek Jamroz, Mateusz Grzegorzek (05z)"<<endl;
	os << "=============================================================================="
		<<endl;
	os << "Encoding: " <<argv[0]<<" [encoding-options] INPUTWAVFILE [OUTPUTVLAKFILE]"<<endl;
	os << "Decoding: " <<argv[0]<<" -d [decoding-options] VLAKFILE [OUTPUTWAVFILE]"<<endl;
	os << "Analyzing: "<<argv[0]<<" -a [analysis-options] VLAKFILE"<<endl;

	os << "general options:"<<endl;
	os << "-d, --decode"<<endl;
	os << "-h, --help"<<endl;
	os << "-a, --analyze"<<endl;
	
	os << "encoding options:"<<endl;
	os << "-p, --predictor=#"<<endl; // predictor type
	os << "-b, --blocksize=#"<<endl;
	os << "-m, --manipulator=#"<<endl; // manipulator type
	os << "-l, --lpcorder=#,#"<<endl;
	os << "--fft-qfactor=#"<<endl;
	os << "--fft-minrelampl=#"<<endl;
//	os << "-e, --encoder=#"<<endl; // encoder type (std, paranoid)
	
	os << "analysis options: (mutually exclusive)"<<endl;
	os << "--residual-distrib" << endl; // rozklad sygnalu residualnego
	os << "--residual-signal" << endl; // wykres sygnalu residualnego
	os << "--original-signal" << endl; // wykres sygnalu oryginalnego (niezmanipulowanego)
	os << "--manipulated-signal" << endl; // wykres sygnalu zmanipulowanego
	os << "--predicted-signal"<< endl; // wykres sygnalu z predyktora (manipulated - residual)
	os << "--predictor-order" << endl; // wykres rzedu predyktora dla metod z adaptacyjnym rzedem
	os << "--info" << endl; // podstawowe informacje o pliku vlak (header + ilosc ramek)
	os << "--compression-ratio" << endl; // wykres stopnia kompresji kodera entropii
	// UWAGA: 'wykres' oznacza ze produkowane sa dane do wykresu
	// aby wykreslic np. w gnuplocie mozna posluzyc sie np.
	// plot "plik.dat" with lines

	os << "predictor types:"<<endl;
	for(int i = 0; predictorName(i) != NULL; ++i)
	{
		os << i <<" : "<<predictorName(i)<<endl;
	}
	
	os <<"manipulator types:"<<endl;
	for(int i = 0; manipName(i) != NULL; ++i)
	{
		os << i <<" : "<<manipName(i)<<endl;
	}

	os << "defaults:"<<endl;
	PrintSettings(os);
		
}

void VLAKFrontend::Run() 
{
#define UNSET_FLAG 0
#define HELP_FLAG 1	
#define DECODE_FLAG 2
#define ANALYZE_FLAG 3
#define RESIDUALS_FLAG 4

#define FFT_QFACTOR 5
#define FFT_MINRELAMPL 6
	int cmd_flag = UNSET_FLAG;
	enum AnalyzeFlag analyze_flag = UNSET;
	int c;
	
	static struct option long_options[] = // musi byc static bo inaczej jest segfault
	{
		{"decode",no_argument,&cmd_flag,DECODE_FLAG},
		{"help",no_argument,&cmd_flag,HELP_FLAG},
		{"analyze",no_argument,&cmd_flag,ANALYZE_FLAG},
		{"blocksize",required_argument,NULL,'b'},
		{"predictor",required_argument,NULL,'p'},
		{"manipulator",required_argument,NULL,'m'},
		{"lpcorder",required_argument,NULL,'l'},
		{"fft-qfactor",required_argument,NULL,FFT_QFACTOR},
		{"fft-minrelampl",required_argument,NULL,FFT_MINRELAMPL},
		{"residual-distrib",no_argument,(int *)&analyze_flag,RESIDUAL_DISTRIB},
		{"residual-signal",no_argument,(int *)&analyze_flag,RESIDUAL},
		{"original-signal",no_argument,(int *)&analyze_flag,ORIGINAL},
		{"manipulated-signal",no_argument,(int *)&analyze_flag,MANIPULATED},
		{"predicted-signal",no_argument,(int *)&analyze_flag,PREDICTED},
		{"predictor-order",no_argument,(int *)&analyze_flag,ORDERS},
		{"info",no_argument,(int *)&analyze_flag,INFO},
		{"compression-ratio",no_argument,(int *)&analyze_flag,CR}
	};
	
	while(1)
	{
		int option_index = 0;
		int tmp = 0;
		int p = -1, m = -1, lmin = -1, lmax = -1;
		int retval = 0;
		c = getopt_long(argc,argv,"dhab:p:m:l:",long_options,&option_index);
#ifdef FRONTEND_DEBUG		
		cerr << c <<endl;
#endif		
		if(c == -1) break;
		switch(c)
		{
			case 0 : /* unreachable */
#ifdef FRONTEND_DEBUG				
				cerr << long_options[option_index].name << endl;
#endif				
				break;
			case 'd' : if(cmd_flag != UNSET_FLAG) 
					   throw BadOptionException("option already specified");
				   cmd_flag = DECODE_FLAG;
				   break;
			case 'h' : if(cmd_flag != UNSET_FLAG) 
					   throw BadOptionException("option already specified");
				   cmd_flag = HELP_FLAG;
				   break;
			case 'a' : if(cmd_flag != UNSET_FLAG) 
					   throw BadOptionException("option already specified");
				   cmd_flag = ANALYZE_FLAG;
				   break;
			case '?' : /* unrecognized short option */
				   throw BadOptionException("unrecognized option");
			case 'b' : 
				   if(optarg != NULL) tmp = atoi(optarg);
#ifdef FRONTEND_DEBUG
				   cerr << tmp <<endl;
#endif
				   if(tmp <= 0 || tmp >= 65535) // cos w tym stylu...
					   throw BadOptionException("invalid blocksize");
				   blockSize = tmp;
				   break;
			case 'p' : 
				   if(optarg != NULL) p = atoi(optarg);
#ifdef FRONTEND_DEBUG				   
				   cerr << p <<endl;
#endif				   
				   if(p < 0 || predictorName(p) == NULL)
					   throw BadOptionException("invalid predictor type");
				   predictor = p;
				   break;
			case 'm' : 
				   if(optarg != NULL) m = atoi(optarg);
#ifdef FRONTEND_DEBUG				   
				   cerr << m <<endl;
#endif				   
				   if(m < 0 || manipName(m) == NULL)
					   throw BadOptionException("invalid manipulator type");
				   manipulator = m;
				   break;

			case 'l' : // #.#
				   if(optarg != NULL) retval = sscanf(optarg,"%d,%d",&lmin,&lmax);
#ifdef FRONTEND_DEBUG				   
				   cerr << lmin <<":"<<lmax<<endl;
#endif				   
				   if(retval != 2 || lmin > lmax || lmax > 32) // z sufitu
					   throw BadOptionException("invalid predictor orders");
				   minOrder = lmin;
				   maxOrder = lmax;
				   break;
				   
			case FFT_QFACTOR:
				   if( optarg != NULL ) retval = sscanf(optarg,"%f", &fft_qfactor);
				   if( retval != 1 || fft_qfactor < 0 )
					   throw BadOptionException("invalid qfactor");
				   break;
				   
			case FFT_MINRELAMPL:
				   if( optarg != NULL ) retval = sscanf(optarg,"%f", &fft_minrelampl);
				   if( retval != 1 || fft_minrelampl < 0 )
					   throw BadOptionException("invalid minrelampl");
				   break;
				   
			default : throw BadOptionException("invalid -"+(char)c); // unreachable
		}
	}
#ifdef FRONTEND_DEBUG
	cerr << "cmd_flag= "<<cmd_flag<<endl;
	cerr << "optind= "<<optind<<endl;
#endif	
	
	inputFileName = argv[optind++];
	outputFileName = inputFileName==NULL?NULL:argv[optind];
	switch(cmd_flag)
	{
		case UNSET_FLAG: /* encode albo help */
				if(argc != 1) { Encode(); return; } 
				// else HELP_FLAG
		case HELP_FLAG: PrintHelp(cerr); return;
		case DECODE_FLAG: Decode(); return;
		case ANALYZE_FLAG: Analyze(analyze_flag); return;
	}
}

int main(int argc, char* argv[0])
{
	VLAKFrontend frontend(argc,argv);
	try 
	{
		frontend.Run();
	}
	catch(exception& e) 
	{
		cerr << "fatal exception!" << endl;
		cerr << e.what() << endl;
		return 1;
	}
	return 0;
}
