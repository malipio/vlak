#include <assert.h>
#include <errno.h>
#include <string.h>
#include "File.h"

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif

CFileException::CFileException() :
exception()
{
	iErrno = errno;
}


CFile::CFile()
{
	iFile = NULL;
}
CFile::CFile(const char* aName,const char* aMode)
{
    Open(aName,aMode);
}

CFile::~CFile()
{
	Close();
}

void CFile::RaiseError() //throw (CFileException&)
{
	throw CFileException();
}
void CFile::RaiseEofError() //throw (CFileException&)
{
	throw CEofException();
}
void CFile::RaiseNotFoundError() //throw (CFileException&)
{
	throw CFileNotFoundException();
}


void CFile::Open(const char* aName,const char* aMode)
{
	if( strcmp(aName, "-") == 0 )
	{
		//Open stdin or stdout
		if( strchr(aMode, 'r') != NULL )
		{
			//read mode - open stdin
			//TODO: will this handle binary data correctly?
			iFile = stdin;
		}
		else
		{
			iFile = stdout;
		}

#ifdef WIN32
		//Change stdin or stdout mode to binary in order to avoid translation of CR-LF
		//It is changed back to text mode in Close.
		//Warning: don't open stdin or stdout more than once at a time
		_setmode(fileno(iFile), _O_BINARY);
#endif
	}
	else
	{
		iFile = fopen(aName,aMode);
			
		if( !iFile )
			RaiseNotFoundError();
	}
}

void CFile::Close()
{
	if( iFile )
	{
		if( iFile != stdin && iFile != stdout )  //dont close standard streams
		{
			fclose(iFile);  //TODO: handle close error
		}
		else
		{
#ifdef WIN32
			//Change stdin or stdout mode back to text.
			_setmode(fileno(iFile), _O_TEXT);
#endif
		}

		iFile = NULL;
	}
}

void CFile::Read(void* aBuf, TFileOffset aStartOffset, TFileOffset aLength)
{
	assert(iFile);

	if( fseek(iFile,aStartOffset,SEEK_SET) )
		RaiseError();

	if( fread(aBuf,1,aLength,iFile) != aLength )
		RaiseEofError();
}

void CFile::ReadNext(void* aBuf, TFileOffset aLength)
{
	assert(iFile);
	if( fread(aBuf,1,aLength,iFile) != aLength )
		RaiseEofError();
}

void CFile::Write(const void* aBuf, TFileOffset aStartOffset, TFileOffset aLength)
{
	assert(iFile);
	if( fseek(iFile,aStartOffset,SEEK_SET) )
		RaiseError();
	if( fwrite(aBuf,1,aLength,iFile) != aLength )
		RaiseError();
}

void CFile::Append(const void* aBuf, TFileOffset aLength)
{
	assert(iFile);
	if( fwrite(aBuf,1,aLength,iFile) != aLength )
		RaiseError();
}
void CFile::Seek(long aOffset, CFile::TSeekMode aMode)
{
	assert(iFile);
    if( fseek(iFile,aOffset,aMode) )
		RaiseError();
}

CFile::TFileOffset CFile::Tell()
{
	assert(iFile);
	return ftell(iFile);
}

bool CFile::Eof()
{
	assert(iFile);
	return (bool)(feof(iFile));
}
