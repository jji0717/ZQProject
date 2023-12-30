#include "ServiceGroupManage.h"

namespace ZQTianShan
{	
	namespace EdgeRM
	{		
		ServiceGroupManage::ServiceGroupManage(void)
		{
			mutex = 0;
		}
		ServiceGroupManage::~ServiceGroupManage(void)
		{
		}
		bool ServiceGroupManage::addSGs(char* buf, int len, char separator)
		{
			bool bRet = false;
			if( AnalyseBuf(buf,len,separator) )	//<------------------ 1.解析字符串
			{
				SimplifySGs();					//<-------------------2.合并化简
//				PrintSGM();						//打印整合后的结果
				bRet = true;
			}
			return bRet;	
		}
		bool ServiceGroupManage::addSGs(int begin, int end)
		{	
			bool bRet = false;
			SGPair TempPair = {begin,end};

			while( !InterlockedCompareExchange(&mutex,1,0) )
			{
				if(TempPair.sgBegin <= TempPair.sgEnd )
				{
					_sgMagane[TempPair.sgBegin] = TempPair;
					bRet = true;
				}
				else
				{
					cout<<"error: begin > end   addSGs(int,int)"<<endl;
				}
				mutex = 0;
				break;
			}
			if( bRet )
			{
				SimplifySGs();				//添加成功则 化简合并
//				PrintSGM();					//打印最后结果
			}

			return bRet;
		}
		bool ServiceGroupManage::addSGs(int sgId)
		{
			bool bRet = true;

			if( find(sgId) )
			{
				bRet = false;				//sgId 已经在某个范围内
			}
			else
			{
				bRet = addSGs(sgId,sgId);	//sgId 不在某个范围内，则添加，并把添加 成功与否 返回给 bRet
			}			
			return bRet;
		}
		bool ServiceGroupManage::remove(int begin, int end)
		{
			bool bRet = true;
			SGPair TempPair = {0,0};
			int flag1 = 0;
			int flag2 = 0;
			map<int, SGPair>::iterator Iter;
			map<int, SGPair>::iterator BegIter;
			map<int, SGPair>::reverse_iterator EndIter;
	
			if( begin > end )
			{
				bRet = false;
			}

			if( !InterlockedCompareExchange(&mutex,1,0) && bRet )
			{
					
				
				Iter = _sgMagane.begin();
				BegIter = Iter;
				EndIter = _sgMagane.rbegin();

				while( BegIter != _sgMagane.end() )
				{
					if( BegIter->second.sgBegin >= begin )
					{
						flag1 = 1;							//begin在左边
						break;
					}
					if( BegIter->second.sgBegin < begin && BegIter->second.sgEnd >= begin )
					{
						flag1 = 2;							//begin 大于等于某个end
						break;
					}
					BegIter++;
				}

				while( EndIter != _sgMagane.rend() )
				{
					if( EndIter->second.sgEnd <= end )
					{
						flag2 = 1;							//end 在右边
						break;
					}
					if( EndIter->second.sgEnd > end )
					{
						if( EndIter->second.sgBegin <= end )
						{
							flag2 = 2;							//end 大于等于某个 begin
							break;
						}
					}
					EndIter++;
				}

				if( flag1 == 1 )
				{
					if( flag2 == 1 )
					{
						for( Iter = BegIter ; Iter!=_sgMagane.end() ;)
						{
							if( Iter->first < EndIter->first )
							{
								Iter = _sgMagane.erase( Iter );
							}
							else if( Iter->first == EndIter->first )
							{
								Iter = _sgMagane.erase( Iter );
								break;
							}
							else
							{
								Iter++;
							}
						}
					}
					if( flag2 == 2 )
					{
						TempPair.sgBegin = end+1;
						TempPair.sgEnd   = EndIter->second.sgEnd;
						
						for( Iter = BegIter ; Iter!=_sgMagane.end() ;)
						{
							if( Iter->first <EndIter->first )
							{
								Iter = _sgMagane.erase( Iter );
							}
							else if( Iter->first == EndIter->first )
							{
								Iter = _sgMagane.erase( Iter );
								break;
							}
							else
							{
								Iter++;
							}
						}
						_sgMagane[end+1] = TempPair;
					}
				}
				if( flag1 == 2 )
				{
					if( flag2 == 1 )
					{
						BegIter->second.sgEnd = begin-1;
						for(Iter = ++BegIter ; Iter!=_sgMagane.end() ;)
						{
							if( Iter->first < EndIter->first )
							{
								Iter = _sgMagane.erase( Iter );
							}
							else if( Iter->first == EndIter->first )
							{
								Iter = _sgMagane.erase( Iter );
								break;
							}
							else
							{
								Iter++;
							}
						}
					}
					if( flag2 == 2 )
					{
						TempPair.sgBegin = end+1;
						TempPair.sgEnd   = EndIter->second.sgEnd;
						BegIter->second.sgEnd = begin-1;
						
						for(Iter = ++BegIter; Iter!=_sgMagane.end() ;)
						{
							if( Iter->first < EndIter->first )
							{
								Iter = _sgMagane.erase( Iter );
							}
							else if( Iter->first == EndIter->first )
							{
								Iter = _sgMagane.erase( Iter );
								break;
							}
							else
							{
								Iter++;
							}
						}
						_sgMagane[end+1] = TempPair;
					}
				}
				mutex = 0;
			}//if

//			PrintSGM();
			return bRet;
		}
		void ServiceGroupManage::reset()
		{
			if( !InterlockedCompareExchange(&mutex,1,0) )
			{
				_sgMagane.clear();
				
				mutex = 0;
			}
//			PrintSGM();
		}
		bool ServiceGroupManage::find(int sgId)
		{
			bool bRet = false;
			map<int, SGPair>::iterator Iter;
			
			while( !InterlockedCompareExchange(&mutex,1,0) )
			{
				Iter = _sgMagane.begin();
				
				while( Iter != _sgMagane.end() )
				{
					if( ( Iter->second.sgBegin <= sgId ) && ( Iter->second.sgEnd >= sgId ) )
					{
//						cout<<Iter->first <<"	{"<<Iter->second.sgBegin<<","<<Iter->second.sgEnd <<"}"<<endl;	//如果找到sgId，则显示找到的范围
						bRet = true;
						break;
					}
					Iter++;
				}
				mutex = 0;
				break;
			}
            return bRet;
		}
		bool ServiceGroupManage::AnalyseBuf(char* buf, int len, char separator)
		{
			bool bRet = true;
			SGPair TempPair = {0,0};
			char *str = new char(sizeof(int));
			int str_len  = 0;
			int index = 0;
			
			while( !InterlockedCompareExchange(&mutex,1,0) )
			{	
				for( index = 0;index <= len;index++ )
				{
					if( (buf[index] <=57 && buf[index] >=48) )		//遇到数字，添加到str末尾				
					{																			
						*(str+str_len) = *(buf+index);
						str_len++;	
					}
					
					if( *(buf+index) == separator )					//遇到 separate 把 str 的值赋给 begin
					{
						TempPair.sgBegin  = atoi(str);
						str_len = 0;
					}
																							
					if( *(buf+index) == '\n' )		//遇到 '\r' 把 str 的值赋给 end ，并尝试把 begin 和 end 写入到 map 中
					{												// index == len 这句是为了防止 因字符串的最后没加"\r\n",而使最后一个数据丢失
						TempPair.sgEnd  = atoi(str);								
						str_len = 0;

						if(TempPair.sgBegin > TempPair.sgEnd )
						{
							bRet = false;
							break;
						}
						memset(str,0,sizeof(int));
						_sgMagane[TempPair.sgBegin] = TempPair;
					}
				}
				mutex = 0;
				break;
			}
			return bRet;
		}
		void ServiceGroupManage::SimplifySGs()
		{
			map<int, SGPair>::iterator PreIter;
			map<int, SGPair>::iterator CurIter;

			CurIter = _sgMagane.begin();
			PreIter	= _sgMagane.begin();
			
			while( !InterlockedCompareExchange(&mutex,1,0) )
			{	
				CurIter++;
				while(CurIter != _sgMagane.end())
				{
					if( (PreIter->second).sgEnd >= (CurIter->second).sgBegin-1 )	// (CurIter->second).sgBegin-1 是为了控制这种情况：{123-456} {457-789}
					{
						if( (PreIter->second).sgEnd < (CurIter->second).sgEnd  )
						{
							(PreIter->second).sgEnd = (CurIter->second).sgEnd;	
						}

						_sgMagane.erase(CurIter);
						CurIter = PreIter;			//化简完一次，则从上一个元素开始，继续进行判断
					}
					PreIter = ( CurIter++ );		
				}
				mutex = 0;
				break;
			}
		}
/*		void ServiceGroupManage::PrintSGM()					//打印出 map 中的内容
		{
			map<int, SGPair>::iterator Iter;
			for(Iter = _sgMagane.begin();Iter != _sgMagane.end();Iter++)
			{
				cout<<Iter->first <<"	{"<<Iter->second.sgBegin<<","<<Iter->second.sgEnd <<"}"<<endl;
			}
			if( _sgMagane.size() == 0 )
			{
				cout<<"All elements have been deleted!"<<endl;
			}
		}
*/
	}//end namespace  EdgeRM
}//end namespace ZQTianShan

