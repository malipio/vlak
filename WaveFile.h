#ifndef WAVEFILE_H
#define WAVEFILE_H

#include <exception>

#include "WaveTypes.h"
#include "File.h"

#include "cwavfile.h" // interfejsy IWavFileReader IWavFileWriter
#include "BlocksProviderFromWaveReader.h"
#include "iblocksprovider.h"

/// Exception thrown by WaveFile classes
class CWaveFileFormatException : public std::exception
{
public:
	enum ErrorCode { GeneralError, NotSupported, NoRiff, NoWav, NoFmt, NoData };
	CWaveFileFormatException(ErrorCode aErrorCode) :
	iErrorCode(aErrorCode)
	{
	}

	/// Reason of the exception
	ErrorCode iErrorCode;

	/// Gets text description of the reason
	const char* Text() const;
};

class CFile;

class CWaveFile
{
public:
	/// Supported wave data formats
	enum { WAVE_FORMAT_PCM = 0x0001, };

	/// This structure represents chunk header in wave file
	struct TChunkHeader
	{
		uint iId;
		uint iSize;

		/// Returns number of padding bytes for this chunk.
		/** Chunks are word (2 bytes) aligned. */
		uint Padding() const;
	};

	/// This structure corresponds to fmt chunk in wave file
	struct TWaveHeader
	{
		TChunkHeader iChunk;
		ushort iFormat;
		ushort iChannels;
		uint iSamplesPerSec;
		uint iAvgBytesPerSec;
		ushort iBlockAlign;
		ushort iBitsPerSample;  //only in PCM
		sshort extrabytes;
		//uint bitspersample;

		uint CalculateAvgBytesPerSec() const;
		ushort CalculateBlockAlign() const;
		ushort SizeToWrite() const
		{
			return 24;
		}
	};

public:
	CWaveFile();
	virtual ~CWaveFile();

	/// Returns header of the wave file
	TWaveHeader& WaveHeader();

	/// Returns header of the wave file
	const TWaveHeader& WaveHeader() const;

	/// Returns number of channels in this file.
	/** 1 - mono, 2 - stereo */
	int ChannelsCount() const;

	/// Retunrs number of samples per second (sampling frequency).
	int SamplingRate() const;

	/// Returns number of bits per sample in the file
	int BitsPerSample() const;

protected:  //not public wave file format definitions
	/// Chunk ids
	enum
	{
		ID_RIFF	= 0x46464952,
		ID_WAVE	= 0x45564157,
		ID_FMT	= 0x20746D66,
		ID_DATA	= 0x61746164,
	};

	/// Riff file header
	typedef TChunkHeader TRiffHeader;

protected:
	/// Throws CWaveFileFormatException exception with given reason code.
	void RaiseError(CWaveFileFormatException::ErrorCode aCode) const;
	void NotSupportedError() const;

protected:
	TRiffHeader iRiffHeader;
	
	TWaveHeader iWaveHeader;

	/// Chunk header of wave data chunk
	TChunkHeader iChunkHeader;

	uint iNumberOfSamples;
	uint iBytesPerSample;
};

inline uint CWaveFile::TChunkHeader::Padding() const
{
	//odd length - 1 padding byte
	return (iSize & 1) ? 1 : 0;
}
inline CWaveFile::TWaveHeader& CWaveFile::WaveHeader()
{
	return iWaveHeader;
}
inline const CWaveFile::TWaveHeader& CWaveFile::WaveHeader() const
{
	return iWaveHeader;
}
inline int CWaveFile::ChannelsCount() const
{
	return iWaveHeader.iChannels;
}

inline int CWaveFile::SamplingRate() const
{
	return iWaveHeader.iSamplesPerSec;
}
inline int CWaveFile::BitsPerSample() const
{
	return iWaveHeader.iBitsPerSample;
}

/// Reads wav file
class CWaveReader : public CWaveFile, public IWavFileReader
{
public:
	CWaveReader();
	~CWaveReader();

	void Open(const char* aFilename);
	void Close();

public:
	/// Reads samples from the wave file
	void GetSamples(void* aBuffer, uint aCount);

	uint ChannelsCount() const;
	uint SamplesCount() const;

	/// Returns number of samples that are left
	uint SamplesLeft() const;

	//wojtek tu byl i smieci nadodawal
	//piotrek tu byl takze
	virtual IBlocksProvider * GetBlocksProvider(int blockSize);

private:
	// a czy przypadkiem nie powinno to byc prywatne/chronione??
	BlocksProviderFromWaveReader MyBlocksProvider;

private:
	CFile iFile;
	/// Number of samples already read
	uint iReadSamples;

	void ReadRiffHeader();
	void ReadWaveHeader();
	void FindWaveData();
};

/// Writes wav file.
class CWaveWriter : public CWaveFile, public IWavFileWriter
{
public:
	CWaveWriter();
	~CWaveWriter();

	void Open(const char* aFilename);
	
	/// Closes the file.
	/** It is important to call this method, because it also writes chunk lengths. */
	void Close();

public:  //from IWavFileWriter
	virtual void WriteBlock(const CBlock& block);
	virtual void WriteHeader(const IInfo& header);

private:
	CFile iFile;

	bool iHeaderWritten;
	CFile::TFileOffset iRiffHeaderSizeOffset;
	CFile::TFileOffset iDataChunkStartOffset, iDataChunkSizeOffset;

	void WriteRiffHeader();

	/// Writes actual size of some chunks (RIFF and DATA)
	void FinalHeadersUpdate();

};

#endif  //WAVEFILE_H
