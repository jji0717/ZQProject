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
			if( AnalyseBuf(buf,len,separator) )	//<------------------ 1.�����ַ���
			{
				SimplifySGs();					//<-------------------2.�ϲ�����
//				PrintSGM();						//��ӡ���Ϻ�Ľ��
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
				SimplifySGs();				//��ӳɹ��� ����ϲ�
//				PrintSGM();					//��ӡ�����
			}

			return bRet;
		}
		bool ServiceGroupManage::addSGs(int sgId)
		{
			bool bRet = true;

			if( find(sgId) )
			{
				bRet = false;				//sgId �Ѿ���ĳ����Χ��
			}
			else
			{
				bRet = addSGs(sgId,sgId);	//sgId ����ĳ����Χ�ڣ�����ӣ�������� �ɹ���� ���ظ� bRet
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
						flag1 = 1;							//begin�����
						break;
					}
					if( BegIter->second.sgBegin < begin && BegIter->second.sgEnd >= begin )
					{
						flag1 = 2;							//begin ���ڵ���ĳ��end
						break;
					}
					BegIter++;
				}

				while( EndIter != _sgMagane.rend() )
				{
					if( EndIter->second.sgEnd <= end )
					{
						flag2 = 1;							//end ���ұ�
						break;
					}
					if( EndIter->second.sgEnd > end )
					{
						if( EndIter->second.sgBegin <= end )
						{
							flag2 = 2;							//end ���ڵ���ĳ�� begin
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
//						cout<<Iter->first <<"	{"<<Iter->second.sgBegin<<","<<Iter->second.sgEnd <<"}"<<endl;	//����ҵ�sgId������ʾ�ҵ��ķ�Χ
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
					if( (buf[index] <=57 && buf[index] >=48) )		//�������֣���ӵ�strĩβ				
					{																			
						*(str+str_len) = *(buf+index);
						str_len++;	
					}
					
					if( *(buf+index) == separator )					//���� separate �� str ��ֵ���� begin
					{
						TempPair.sgBegin  = atoi(str);
						str_len = 0;
					}
																							
					if( *(buf+index) == '\n' )		//���� '\r' �� str ��ֵ���� end �������԰� begin �� end д�뵽 map ��
					{												// index == len �����Ϊ�˷�ֹ ���ַ��������û��"\r\n",��ʹ���һ�����ݶ�ʧ
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
					if( (PreIter->second).sgEnd >= (CurIter->second).sgBegin-1 )	// (CurIter->second).sgBegin-1 ��Ϊ�˿������������{123-456} {457-789}
					{
						if( (PreIter->second).sgEnd < (CurIter->second).sgEnd  )
						{
							(PreIter->second).sgEnd = (CurIter->second).sgEnd;	
						}

						_sgMagane.erase(CurIter);
						CurIter = PreIter;			//������һ�Σ������һ��Ԫ�ؿ�ʼ�����������ж�
					}
					PreIter = ( CurIter++ );		
				}
				mutex = 0;
				break;
			}
		}
/*		void ServiceGroupManage::PrintSGM()					//��ӡ�� map �е�����
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

