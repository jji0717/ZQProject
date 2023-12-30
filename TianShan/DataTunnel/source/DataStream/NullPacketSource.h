// NullPacketSource.h: interface for the NullPacketSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NULLPACKETSOURCE_H__A583AA3D_CC55_4EB4_9216_13D654AC9506__INCLUDED_)
#define AFX_NULLPACKETSOURCE_H__A583AA3D_CC55_4EB4_9216_13D654AC9506__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataSource.h"

namespace DataStream {

class NullPacketSource: public DataSource
{
public:
	NullPacketSource();
	virtual ~NullPacketSource();

	virtual bool init()
	{
		return true;
	}

	virtual bool nextBlock(BufferBlock& block);
	virtual void newCycle();

};

} // namespace DataStream {

#endif // !defined(AFX_NULLPACKETSOURCE_H__A583AA3D_CC55_4EB4_9216_13D654AC9506__INCLUDED_)
