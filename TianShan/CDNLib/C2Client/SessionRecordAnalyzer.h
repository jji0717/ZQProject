#ifndef __c2client_session_record_analyzer_h__
#define __c2client_session_record_analyzer_h__

#include "SessionDataRecorder.h"

class SessionRecordAnalyzer
{
public:
	SessionRecordAnalyzer( SessionDataRecorder& rec );
	virtual ~SessionRecordAnalyzer(void);

public:

	void	show();


private:
	SessionDataRecorder&	mRecorder;

};

#endif//__c2client_session_record_analyzer_h__

