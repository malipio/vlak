#include <assert.h>
#include <memory.h>
#include "File.h"
#include "WaveFile.h"

//enum ErrorCode { GeneralError, NotSupported, NoRiff, NoWav, NoFmt, NoData };
static const char* WaveFileFormatExceptionTexts[] = {
	"General error",
	"Not supported wave file format",
	"No RIFF section in the file",
	"No WAV section in the file",
	"No FMT section in the file",
	"No DATA section in the file",
};

const char* CWaveFileFormatException::Text() const
{
	return WaveFileFormatExceptionTexts[iErrorCode];
}

//////////////////////////////
// CWaveFile class
//////////////////////////////

uint CWaveFile::TWaveHeader::CalculateAvgBytesPerSec() const
{
	return iSamplesPerSec * iChannels * (iBitsPerSample >> 3);
}
ushort CWaveFile::TWaveHeader::CalculateBlockAlign() const
{
	return iChannels * (iBitsPerSample >> 3);
}

CWaveFile::CWaveFile()
{
}

CWaveFile::~CWaveFile()
{
}

void CWaveFile::RaiseError(CWaveFileFormatException::ErrorCode aCode) const
{
	throw CWaveFileFormatException(aCode);
}

void CWaveFile::NotSupportedError() const
{
	RaiseError(CWaveFileFormatException::NotSupported);
}


//////////////////////////////
// CWaveReader class
//////////////////////////////

CWaveReader::CWaveReader() :
MyBlocksProvider(this), iFile(), iReadSamples(0)
{
}

CWaveReader::~CWaveReader()
{
	iFile.Close();
}


void CWaveReader::Open(const char* aFilename)
{
	iFile.Open(aFilename,"rb");

	ReadRiffHeader();
	ReadWaveHeader();
	FindWaveData();

	iReadSamples = 0;
}

void CWaveReader::Close()
{
	iFile.Close();
}

void CWaveReader::ReadRiffHeader()
{
	iFile.ReadNext(&iRiffHeader,sizeof(TRiffHeader));
	
	if( iRiffHeader.iId != ID_RIFF )
		RaiseError(CWaveFileFormatException::NoRiff);
}

void CWaveReader::ReadWaveHeader()
{
	//WAVE section
	uint wavetag;
	iFile.ReadNext(&wavetag, sizeof(uint));
	
	if( wavetag != ID_WAVE )
		RaiseError(CWaveFileFormatException::NoWav);

	//now there should to be fmt section
	
	iFile.ReadNext(&iWaveHeader.iChunk,sizeof(TChunkHeader));
	if( iWaveHeader.iChunk.iId != ID_FMT )
		RaiseError(CWaveFileFormatException::NoFmt);

	if( iWaveHeader.iChunk.iSize > sizeof(TWaveHeader) - sizeof(TChunkHeader) )
		RaiseError(CWaveFileFormatException::NotSupported);

	//read rest of the header, first field is format
	iFile.ReadNext(&iWaveHeader.iFormat,iWaveHeader.iChunk.iSize);
	
	//only PCM supported
	if( iWaveHeader.iFormat != WAVE_FORMAT_PCM )
		RaiseError(CWaveFileFormatException::NotSupported);

	//only stereo files supported
	if( iWaveHeader.iChannels != 2 )
		RaiseError(CWaveFileFormatException::NotSupported);

	if( iWaveHeader.iBitsPerSample != 16 )
		RaiseError(CWaveFileFormatException::NotSupported);

	iBytesPerSample = (iWaveHeader.iChannels * (iWaveHeader.iBitsPerSample>>3) );
}

void CWaveReader::FindWaveData()
{
	while( !iFile.Eof() )
	{
		memset(&iChunkHeader,0,sizeof(TChunkHeader));
		iFile.ReadNext(&iChunkHeader,sizeof(TChunkHeader));
		if( iChunkHeader.iId == ID_DATA )
		{
			//we found data chunk
			iNumberOfSamples = iChunkHeader.iSize / iBytesPerSample;
			break;
		}
		else
		{
			//some other section - ignore it
			if( iChunkHeader.iSize <= 0 )
				RaiseError(CWaveFileFormatException::NoData);

			iFile.Seek(iChunkHeader.iSize + iChunkHeader.Padding() );
		}
	}

	if( iFile.Eof() )
	{
		RaiseError(CWaveFileFormatException::NoData);
	}
}

void CWaveReader::GetSamples(void* aBuffer, uint aCount) //wczytuje tak naprawde aCount sampli * iWaveHeader.iChannels
{
	if( iBytesPerSample != 4 )
		NotSupportedError();

	assert(iReadSamples + aCount <= iNumberOfSamples);

	iFile.ReadNext(aBuffer,aCount * iBytesPerSample);
	iReadSamples += aCount;

	//assert( !iFile.Eof() );
}

uint CWaveReader::ChannelsCount() const
{
	return CWaveFile::ChannelsCount();
}

uint CWaveReader::SamplesCount() const
{
	return iNumberOfSamples;
}

uint CWaveReader::SamplesLeft() const
{
	return iNumberOfSamples - iReadSamples;
}

//metoda wojtka
IBlocksProvider * CWaveReader::GetBlocksProvider(int blockSize)
{
	MyBlocksProvider.iBlockSize=blockSize;
	return & MyBlocksProvider;
}

//////////////////////////////
// CWaveWriter class
//////////////////////////////

CWaveWriter::CWaveWriter() :
iFile(), iHeaderWritten(false)
{
}

CWaveWriter::~CWaveWriter()
{
	iFile.Close();
}


void CWaveWriter::Open(const char* aFilename)
{
	iFile.Open(aFilename,"wb");
	iHeaderWritten = false;
}

void CWaveWriter::Close()
{
	FinalHeadersUpdate();
	iFile.Close();
}

void CWaveWriter::WriteBlock(const CBlock& block)
{
	//todo: test, if block has apropriate format (channels, frequency)
	assert(iHeaderWritten);

	int numChannels = block.GetInfo().GetNumOfChannels();
	int numSamples = block.GetBlockSize();
	
	//interleave samples from all channels
	for(int sample=0 ; sample<numSamples ; ++sample)
		for(int ch=0 ; ch<numChannels ; ++ch)
		{
			//todo: what with 8-bit samples?
			iFile.Append( &block.GetSamples(ch)[sample], sizeof(CSample) );
		}
}

void CWaveWriter::WriteHeader(const IInfo& header)
{
	assert( !iHeaderWritten );

	WriteRiffHeader();

	//WAVE
	uint wavetag = ID_WAVE;
	iFile.Append(&wavetag,sizeof(uint));

	//prepare wave header (fmt chunk)
	iWaveHeader.iChunk.iId = ID_FMT;
	iWaveHeader.iChunk.iSize = iWaveHeader.SizeToWrite() - 8;  //does not include iId and iSize

	iWaveHeader.iFormat = WAVE_FORMAT_PCM;
	iWaveHeader.iBitsPerSample = header.GetBitsPerSample();
	iWaveHeader.iChannels = header.GetNumOfChannels();
	iWaveHeader.iSamplesPerSec = header.GetSamplingFrequency();
	iWaveHeader.iAvgBytesPerSec = iWaveHeader.CalculateAvgBytesPerSec();
	iWaveHeader.iBlockAlign = iWaveHeader.CalculateBlockAlign();

	iFile.Append(&iWaveHeader, iWaveHeader.SizeToWrite() );

	//prepare data chunk
	iChunkHeader.iId = ID_DATA;
	iChunkHeader.iSize = 0;  //will be filled later

	iDataChunkStartOffset = iFile.Tell();

	iFile.Append(&iChunkHeader, sizeof(TChunkHeader));

	iDataChunkSizeOffset = iFile.Tell() - sizeof(uint);

	iHeaderWritten = true;
}

void CWaveWriter::WriteRiffHeader()
{
	//riff header is always first
	iRiffHeader.iId = ID_RIFF;
	iRiffHeader.iSize = 0;  //will be filled later
	iFile.Append(&iRiffHeader,sizeof(iRiffHeader));

	iRiffHeaderSizeOffset = iFile.Tell() - sizeof(uint);
}

void CWaveWriter::FinalHeadersUpdate()
{
	CFile::TFileOffset fileSize = iFile.Tell();
	uint size;

	//riff size
	size = fileSize - sizeof(TRiffHeader);
	iFile.Write(&size,iRiffHeaderSizeOffset,sizeof(uint));

	//data size
	size = fileSize - (iDataChunkStartOffset + sizeof(TChunkHeader));
	iFile.Write(&size,iDataChunkSizeOffset,sizeof(uint));
}


