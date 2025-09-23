#pragma once

class CRiceCoderTests
{
public:
	CRiceCoderTests(void);
	~CRiceCoderTests(void);

	bool TestAll();
	bool Test8Bit();
	bool Test16Bit();
	bool Test32Bit();
};
