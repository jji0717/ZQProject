
#include <math.h>
#include <iostream>
#include "SessionRecordAnalyzer.h"

SessionRecordAnalyzer::SessionRecordAnalyzer( SessionDataRecorder& rec)
:mRecorder(rec)
{
}

SessionRecordAnalyzer::~SessionRecordAnalyzer(void)
{
}

#define ABS(x) ((x)>0?(x):-(x))
void SessionRecordAnalyzer::show()
{
	SessionDataRecorder::RECORDS& recs = mRecorder.getAllRecords();
	SessionDataRecorder::RECORDS::const_iterator it = recs.begin();
	for( ; it != recs.end(); it ++ )
	{
		int64 avg2 = 0;
		int64 avg = 0 ;
		int64 total = 0 ;
		int64 firstTimeStamp = 0;
		int64 lastTimeStamp = 0;
		int64 count = 0 ;
		int64 lastGotBytes = 0;
		for( int i = 0 ;i < 2 ; i ++ )
		{
			DataRecord::RecordInfo* pRecInfo = it->second->getDataHeader();
			while( pRecInfo )
			{
				for( size_t offset = 0 ; offset < pRecInfo->dataSize() ; offset+= sizeof(HttpSessionDataRecordInfo) )
				{
					HttpSessionDataRecordInfo* pDataInfo = (HttpSessionDataRecordInfo*)(pRecInfo->buffer+offset);
					if( !(i % 2) )
					{
						total += pDataInfo->size;
						if( firstTimeStamp <= 0 )
							firstTimeStamp = pDataInfo->time;
						lastTimeStamp = pDataInfo->time;
					}
					else
					{
						if( lastGotBytes <= 0 )
						{
							lastGotBytes = avg - pDataInfo->size;
							lastTimeStamp = pDataInfo->time;
						}
						else
						{
							int64 delta = ABS(lastTimeStamp-pDataInfo->time);
							if( delta >= 1000 )
							{
								delta = ABS(lastTimeStamp-pDataInfo->time);
								int64 rate = lastGotBytes * 8000 / delta;
								
								count ++ ;							
								delta = ABS( avg - rate );
								total +=(int64)sqrt((double)(delta*delta));
								
							
								lastGotBytes = pDataInfo->size;
								lastTimeStamp = pDataInfo->time;
							}
							else
							{
								lastGotBytes += pDataInfo->size;
							}							
						}
					}
				}
				pRecInfo = pRecInfo->next;
			}
			if( i == 0)
			{
				int64 timeDelta = lastTimeStamp - firstTimeStamp;
				if( timeDelta > 0 )
				{
					avg = total * 8000 / timeDelta;
				}
				total = 0;
			}
			else
			{
				if(count>0)
				{
					avg2 = (int64)sqrt((double)(total / count));
				}
			}			
		}
		std::cout<<"session["<<it->first<<"] avg["<<avg<<"] avg2["<<avg2<<"]"<<std::endl;		
	}
}

