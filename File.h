#ifndef FILE_H
#define FILE_H

#include <exception>
#include <stdio.h>

class CFileException : public std::exception
{
public:
	/// Creates exception, gets current value of errno
	CFileException();
	/// Creates exception with specified value of errno
	CFileException(int aErrno);

	/// Errno value - reason of the exception
	int iErrno;
};

class CFileNotFoundException : public CFileException
{
public:
	CFileNotFoundException() : CFileException()
	{
	}
};

class CEofException : public CFileException
{
public:
	CEofException() : CFileException()
	{
	}
};

/// Class enapsulating a file in an object
class CFile
{
public:  //Construction
	CFile();
	CFile(const char* aName,const char* aMode);
	~CFile();
	void Open(const char* aName,const char* aMode);
	void Close();

public:  //Operations
	typedef unsigned int TFileOffset;
	void Read(void* aBuf, TFileOffset aStartOffset, TFileOffset aLength);
	void ReadNext(void* aBuf, TFileOffset aLength);
	void Write(const void* aBuf, TFileOffset aStartOffset, TFileOffset aLength);
	void Append(const void* aBuf, TFileOffset aLength);

	bool Eof();
	
	enum TSeekMode
	{
		SeekStart = SEEK_SET,
		SeekRelative = SEEK_CUR,
		SeekEnd = SEEK_END,
	};
	void Seek(long aOffset, TSeekMode aMode = SeekRelative);

	TFileOffset Tell();

protected:
	void RaiseError(); //throw (CFileException&);
	void RaiseEofError(); //throw (CFileException&);
	void RaiseNotFoundError(); //throw (CFileException&);
private:
	FILE *iFile;
};

#endif  //FILE_H
