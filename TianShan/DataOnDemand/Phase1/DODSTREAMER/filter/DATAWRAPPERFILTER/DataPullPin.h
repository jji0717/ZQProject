
#ifndef DATAPULLPIN_H
#define DATAPULLPIN_H

#include "pullpin3.h"
//#include "common.h"

class CDataInputPin;

class CDataPullPin : public CPullPin //CAMThread
{
public:
protected:
private:
	CDataInputPin * m_pPin;
public:
	CDataPullPin(CDataInputPin* pPin);
	virtual ~CDataPullPin();

	HRESULT SetSubChannelList( SubChannelVector &pSubChannellist );

	HRESULT SendFlag( BYTE byFlag );
	void SortBufferAndDelive();
	void InitSortBuffer();
	// -pure-
	// Process sample and pass sample to output pin.
	HRESULT Receive(IMediaSample*pSample);
	HRESULT EndOfStream(void);
	void OnError(HRESULT hr);
	HRESULT BeginFlush();
	HRESULT EndFlush();
};

#endif