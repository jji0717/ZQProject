#ifndef _SDPCONTENT_H_
#define _SDPCONTENT_H_

#include "../Common.h"

typedef struct SDPRequestContent
{
	int32	iCtrlNum;
	string	strProvider_id;		//<provider-id>
	string	strAsset_id;		//<asset-id>
	string	strRange;			//<range>
}SDPRequestContent;

typedef list<SDPRequestContent> SDPRequestContentVec;

typedef struct SDPResponseContent
{
	string	strProtocol;		//<protocol>
	string	strHost;			//<host>
	uint32	uPort;				//<port>
	string	strStreamhandle;	//<streamhandle>
}SDPResponseContent;

class FindByCtlrNum
{
public:
	FindByCtlrNum(int32 iCtrlNum):_iCtrlNum(iCtrlNum){}

	bool operator ()(SDPRequestContent &pSDP)
	{
		if (_iCtrlNum == pSDP.iCtrlNum)
			return true;
		else
			return false;
	}
private:
	int32 _iCtrlNum;
};

#endif