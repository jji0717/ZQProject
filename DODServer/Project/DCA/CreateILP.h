#pragma once

#if !defined(JERROY_ZHENAN_DISCUSS_RESULT_2006_02_28)
#define JERROY_ZHENAN_DISCUSS_RESULT_2006_02_28


class CCreateILP
{
public:
	CCreateILP(void);
	~CCreateILP(void);

	CString CreateILPFile(CString sFilename,int nDatyType);
};
#endif