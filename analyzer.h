// $Id: analyzer.h,v 1.5 2006/01/11 23:34:48 pmalinow Exp $
// klasa analizatora dla VLAKow
#ifndef ANALYZER_H
#define ANALYZER_H
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <string.h>
#include <assert.h>
#include <cwalk.h>

#include "streamops.h" // przedefiniowane operatory<<
#include "idecoder.h"
#include "iencoder.h"
#include "WaveFile.h"
#include "cfirpredictor.h"
#include "clpcpredictor.h"
#include "cwaveletpredictor.h"

using namespace std;

class CResidualsProvider {
	private:
		IFrame *frame;
		CLPCRiceFrame *lpcFrame;
		CWaveletCompressedFrame *waveletFrame;
		CFIRCompressedFrame *firFrame;
	public:
		CResidualsProvider(IFrame* iFrame) 
		{
			frame = iFrame;
			lpcFrame = dynamic_cast<CLPCRiceFrame *>(frame);
			waveletFrame = dynamic_cast<CWaveletCompressedFrame *>(frame);
			firFrame = dynamic_cast<CFIRCompressedFrame *>(frame);
		}

		CExtSample *GetResiduals(char chNum)
		{
			if(lpcFrame != NULL)
			{
				return lpcFrame->order[chNum]==0? NULL : lpcFrame->GetResiduals(chNum);
			}
			if(waveletFrame != NULL)
			{
				return waveletFrame->GetResiduals(chNum);
			}
			if(firFrame != NULL)
			{
				return firFrame->GetResiduals(chNum);
			}
			// TODO: kod dla FFTa
			return NULL;
		}

		int Length(char chNum)
		{
			if(lpcFrame != NULL)
			{
				return lpcFrame->order[chNum] == 0? 0: 
					lpcFrame->GetBlockSize()-lpcFrame->order[chNum];
			}
			if(waveletFrame != NULL)
			{
				return waveletFrame->GetBlockSize();
			}
			if(firFrame != NULL)
			{
				return firFrame->GetBlockSize()-3;
			}
			// TODO: kod dla FFTa
			return 0;
		}
};

class COrderProvider {
	IFrame *frame;
	CLPCRiceFrame *lpcFrame;
	public:
		COrderProvider(IFrame *iFrame)
		{
			frame = iFrame;
			lpcFrame = dynamic_cast<CLPCRiceFrame *>(frame);
		}

		int GetOrder(char chNum)
		{
			if(lpcFrame != NULL)
			{
				return lpcFrame->order[chNum];
			}
			// TODO: kod dla FFT (?)
			return 0;
		}
};

class VLAKAnalyzer {
	private:
		string namePrefix; // np. dla tests/x.vlak -> tests/x.vlak.
		string inputFileName;
		//pliki wynikowe otrzymuja suffixy :)
		//np tests/x.vlak.f000
	public:
		VLAKAnalyzer(const string& inputFileName) 
		{
			this->inputFileName = inputFileName;
			char *tmp = strdup(inputFileName.c_str());
			size_t dirnameLength;
			cwk_path_get_dirname(tmp, &dirnameLength);
			namePrefix.assign(tmp, dirnameLength);
			free(tmp);
			namePrefix += "/";
//			namePrefix = inputFileName+"."; // TODO
		}
		class IDataPrinter {
			public:
			virtual void operator()(ostream& os, IFrame& frame, char chNum) const {};
			virtual void operator()(ostream& os, CBlock& block, char chNum) const {};
		};

		class PrintResidualDistribution : public IDataPrinter{ // rozklad, a nie sygnal
			public:
			virtual void operator()(ostream& os, IFrame& frame, char chNum) const
			{
				CResidualsProvider prov(&frame);
				map<CExtSample,int> tab;
				double EX = 0.0;
				if(prov.Length(chNum) == 0) return; // NO RESIDUAL SIGNAL
				
				for(int i = 0; i< prov.Length(chNum); ++i)
					tab[prov.GetResiduals(chNum)[i]] = 0; // init
				
				for(int i = 0; i< prov.Length(chNum); ++i)
				{
					tab[prov.GetResiduals(chNum)[i]]++;
					EX += prov.GetResiduals(chNum)[i];
				}

				// wartosc oczekiwana wg definicji dyskretnej
			//	EX /= prov.Length(chNum);
			
			//	os <<"plot '-' title 'f'";
			//	os <<", '-' title 'EX' with impulses";
			//	os<<endl;
				
				for(int i = 0; i< prov.Length(chNum); ++i)
				{
					os << prov.GetResiduals(chNum)[i]<<
						"\t"<<tab[prov.GetResiduals(chNum)[i]]<<endl;
				}

			//	os << endl << "e"<<endl<<EX<<" 6"<<endl;
			//	os << endl << "e"<<endl;
			//	os<<"pause -1 'waiting....'"<<endl;
			}
		};
		
		class PrintResidualSignal : public IDataPrinter
		{
			public:
			virtual void operator()(ostream& os, IFrame& frame, char chNum) const
			{
				CResidualsProvider prov(&frame);
				if(prov.Length(chNum) == 0) return; // NO RESIDUAL SIGNAL
				
				for(int i = 0; i< prov.Length(chNum); ++i)
				{
					// sygnal residualny jest krotszy niz oryginalny
					os << i << "\t" << prov.GetResiduals(chNum)[i]<< endl;
				}
			}
		};

		class PrintOriginalSignal : public IDataPrinter 
		{
			public:
			virtual void operator()(ostream& os, CBlock& block, char chNum) const
			{
				IChannelManipulator *manip = IChannelManipulator::
					CreateInstance(block.GetManipulatorUID());
				CBlock restored = manip->Restore(block);
				delete manip;
				for(int i = 0; i < restored.GetBlockSize(); ++i)
				{
					os << i << "\t" << restored.GetSamples(chNum)[i] << endl;
				}

			}
				
		};
		
		class PrintManipulatedSignal : public IDataPrinter 
		{
			public:
			virtual void operator()(ostream& os, CBlock& block, char chNum) const
			{
				for(int i = 0; i < block.GetBlockSize(); ++i)
				{
					os << i << "\t" << block.GetSamples(chNum)[i] << endl;
				}

			}
				
		};

		class PrintPredictedSignal : public IDataPrinter // roznica miedzy org. a res.
		{
			public:
			virtual void operator()(ostream& os, IFrame& frame, char chNum) const
			{
				// TODO
				
			}
			
		};

		class PrintPredictorOrder : public IDataPrinter // roznica miedzy org. a res.
		{
			public:
			virtual void operator()(ostream& os, IFrame& frame, char chNum) const
			{
				COrderProvider prov(&frame);
				os << prov.GetOrder(chNum) << endl;				
			}
			
		};
		
		class PrintCompressionRatio : public IDataPrinter // efektywnosc RiceCodera
		{
			// TODO: zastanowic czy nie zmienic sposoby liczenia
			// tak zeby wyliczacz rzeczywiste CR a nie tylko CR kodera
			public:
			virtual void operator()(ostream& os, IFrame& frame, char chNum) const
			{
				COrderProvider prov(&frame);
				ICompressedFrame *cFrame = dynamic_cast<ICompressedFrame *>(&frame);
				assert(cFrame != NULL);
				double cr = (double)cFrame->GetDataStream().Size()/
						(double)cFrame->GetCompressedDataStream().Size();
				os << cr << endl;				
			}
			
		};


	protected:
		string CreateFileName(const string& analysisName)
		{
			return namePrefix+analysisName+".dat";
		}
		string CreateFileName(const string& analysisName,int chNum)
		{
			return namePrefix+analysisName+".ch"+toString(chNum,10,1)+".dat";
		}
		string CreateFileName(const string& analysisName,int chNum,int frameNum)
		{
			return namePrefix+analysisName+".f"+toString(frameNum,10,6)+
				".ch"+toString(chNum,10,1)+".dat";
		}
		
		void AnalyzeEachFrame(const IDataPrinter& dataPrinter,const string& analysisName) 
		{
			CVLAKFile vlak(inputFileName,"rb");
			CVLAKHeader header = vlak.ReadHeader();
			for(int frameNum = 0;;++frameNum)
			{
				ICompressedFrame *cFrame = vlak.ReadNextFrame(); // new
				if(vlak.Eof()) break; // koniec pliku
				// teraz trzeba odtworzyc kompresor na podstawie ramki
				IEntropyCompressor *compressor = cFrame->GetCompressor(); // new
				
				IFrame * frame = compressor->DecompressFrame(*cFrame); // new
				IPredictor *predictor = frame->GetPredictor();
				CBlock block = predictor->DecodeFrame(*frame);
//				IChannelManipulator *manip = IChannelManipulator::
//					CreateInstance(block.GetManipulatorUID());
//				block = manip->Restore(block);
//				delete manip;

				delete predictor; 
				for(char chNum = 0; chNum < 
						frame->GetInfo().GetNumOfChannels();++chNum)
				{
					string fname = CreateFileName(analysisName,chNum,frameNum);
					cerr << fname << endl;
					ofstream fos(fname.c_str());
					dataPrinter(fos,*frame,chNum);
					dataPrinter(fos,block,chNum);
				}
				delete compressor;
				delete frame; // cFrame
			}
		}

		void AnalyzeEachChannel(const IDataPrinter& dataPrinter,const string& analysisName) 
		{
			CVLAKFile vlak(inputFileName,"rb");
			CVLAKHeader header = vlak.ReadHeader();
			vector<ofstream *> fstreams;
			for(char chNum = 0; chNum < header.GetNumOfChannels(); ++chNum)
			{
				fstreams.push_back(new ofstream( 
							CreateFileName(analysisName,chNum).c_str()));
			}
			
			for(int frameNum = 0;;++frameNum)
			{
				ICompressedFrame *cFrame = vlak.ReadNextFrame(); // new
				if(vlak.Eof()) break; // koniec pliku
				// teraz trzeba odtworzyc kompresor na podstawie ramki
				IEntropyCompressor *compressor = cFrame->GetCompressor(); // new
				
				IFrame * frame = compressor->DecompressFrame(*cFrame); // new
				IPredictor *predictor = frame->GetPredictor();
				CBlock block = predictor->DecodeFrame(*frame);
				
				delete predictor; 
				for(char chNum = 0; chNum < 
						frame->GetInfo().GetNumOfChannels();++chNum)
				{
					dataPrinter(*fstreams[chNum],*frame,chNum);
					dataPrinter(*fstreams[chNum],block,chNum);
				}
				delete compressor;
				delete frame; // cFrame
			}

			for(unsigned int i = 0; i < fstreams.size(); i++)
				delete fstreams[i];
			
		}
		
		void AnalyzeAllFrames(const IDataPrinter& dataPrinter,const string& analysisName) 
		{
			CVLAKFile vlak(inputFileName,"rb");
			CVLAKHeader header = vlak.ReadHeader();
			string fname = CreateFileName(analysisName);
			cerr << fname << endl;
			ofstream fos(fname.c_str());
			for(int frameNum = 0;;++frameNum)
			{
				ICompressedFrame *cFrame = vlak.ReadNextFrame(); // new
				if(vlak.Eof()) break; // koniec pliku
				// teraz trzeba odtworzyc kompresor na podstawie ramki
				IEntropyCompressor *compressor = cFrame->GetCompressor(); // new
				
				IFrame * frame = compressor->DecompressFrame(*cFrame); // new
				IPredictor *predictor = frame->GetPredictor();
				CBlock block = predictor->DecodeFrame(*frame);

				delete predictor; 
				for(char chNum = 0; chNum < 
						frame->GetInfo().GetNumOfChannels();++chNum)
				{
					dataPrinter(fos,*frame,chNum);
					dataPrinter(fos,block,chNum);
				}
				delete compressor;
				delete frame; // cFrame
			}
		}
	public:
		void ProduceResidualDistribution() 
		{
			AnalyzeEachFrame(PrintResidualDistribution(),"res_dist");
		}

		void ProduceOriginalSignal()
		{
			AnalyzeEachFrame(PrintOriginalSignal(),"orig");
		}
		
		void ProduceManipulatedSignal()
		{
			AnalyzeEachFrame(PrintManipulatedSignal(),"manip");
		}
		
		void ProduceResidualSignal()
		{
			AnalyzeEachFrame(PrintResidualSignal(),"resid");
		}

		void ProducePredictorOrders()
		{
			AnalyzeEachChannel(PrintPredictorOrder(),"orders");
		}

		void ProduceCompressionRatio()
		{
			AnalyzeAllFrames(PrintCompressionRatio(),"cr");
		}

		void ProduceInfo()
		{
			CVLAKFile vlak(inputFileName,"rb");
			CVLAKHeader header = vlak.ReadHeader();
			int frameNum = 0;
			for(;;++frameNum)
			{
				ICompressedFrame *cFrame = vlak.ReadNextFrame(); // new
				if(vlak.Eof()) break; // koniec pliku
				delete cFrame;
			}
			ofstream fos(CreateFileName("info").c_str());
			fos << header;
			fos << "frames= "<<frameNum<<endl;
		}

};
#endif
