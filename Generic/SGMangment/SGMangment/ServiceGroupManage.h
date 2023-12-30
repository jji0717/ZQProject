#ifndef __MANAGE_DFKDFEIFJE88_393_EW_SERVICEGROUP__34942
#define __MANAGE_DFKDFEIFJE88_393_EW_SERVICEGROUP__34942
#include <iostream>
#include <cstring>
#include <windows.h>
#include <map>
///实现一个线程安全的ServiceGroupManage类, 可以添加查找ServiceGroup///

namespace ZQTianShan
{
	
	namespace EdgeRM
	{
		using namespace std;		

		class ServiceGroupManage
		{
		public:
			ServiceGroupManage(void);
		public:
			~ServiceGroupManage(void);
		public:

			/* 
			Buf: 类似于这样的
			   100-200 \r\n
			   300-500 \r\n
			   600-900 \r\n
			*/
			/// add serviceGroup
			///@param buf		  serviceGroup pair 存储的Buf
			///@param len		  buf长度
			///@param separator   serviceGroup pair分隔符, 要求解析出来 end >= begin
			///@return bool       成功返回 true, 失败返回 false ，注意异常情况 
			bool addSGs(char* buf, int len, char separator);

			/// add serviceGroup by sgBegin and sgEnd
			///@param	begin ServiceGroup的起始数
			///@param	end   ServiceGroup的结束数
			///@return bool       成功返回 true, 失败返回 false ，注意异常情况
			bool addSGs(int begin, int end);

			/// add serviceGroup by sgId
			///@param	begin ServiceGroup的起始数
			///@param	end   ServiceGroup的结束数
			///@return bool       成功返回 true, 失败返回 false ，注意异常情况
			bool addSGs(int sgId);

			/// remove serviceGroup pair
			///@param	begin ServiceGroup的起始数
			///@param	end   ServiceGroup的结束数
			///@return bool       成功返回 true, 失败返回 false ，注意异常情况
			bool remove(int begin, int end);

			/// 清空所有Servicegroup
			void reset();

			///find sgId 是否存在
			///@return bool   存在 返回true,不存在 返回false
			bool find(int sgId);

			//解析字符串 生成_sgMagane
			bool AnalyseBuf(char* buf, int len, char separator);

			//化简整合，使重合部分，连接一起
			void SimplifySGs();
			
			//打印出_sgMagane中的内容
//			void PrintSGM();
		protected:

			typedef struct  
			{
				int sgBegin;
				int sgEnd;
			}SGPair;

			typedef map<int, SGPair>SGManage;
			SGManage _sgMagane;
		private:
			long volatile mutex;
		};
	}//end namespace  EdgeRM
}//end namespace ZQTianShan

#endif
