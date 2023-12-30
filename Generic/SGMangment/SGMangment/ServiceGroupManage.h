#ifndef __MANAGE_DFKDFEIFJE88_393_EW_SERVICEGROUP__34942
#define __MANAGE_DFKDFEIFJE88_393_EW_SERVICEGROUP__34942
#include <iostream>
#include <cstring>
#include <windows.h>
#include <map>
///ʵ��һ���̰߳�ȫ��ServiceGroupManage��, ������Ӳ���ServiceGroup///

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
			Buf: ������������
			   100-200 \r\n
			   300-500 \r\n
			   600-900 \r\n
			*/
			/// add serviceGroup
			///@param buf		  serviceGroup pair �洢��Buf
			///@param len		  buf����
			///@param separator   serviceGroup pair�ָ���, Ҫ��������� end >= begin
			///@return bool       �ɹ����� true, ʧ�ܷ��� false ��ע���쳣��� 
			bool addSGs(char* buf, int len, char separator);

			/// add serviceGroup by sgBegin and sgEnd
			///@param	begin ServiceGroup����ʼ��
			///@param	end   ServiceGroup�Ľ�����
			///@return bool       �ɹ����� true, ʧ�ܷ��� false ��ע���쳣���
			bool addSGs(int begin, int end);

			/// add serviceGroup by sgId
			///@param	begin ServiceGroup����ʼ��
			///@param	end   ServiceGroup�Ľ�����
			///@return bool       �ɹ����� true, ʧ�ܷ��� false ��ע���쳣���
			bool addSGs(int sgId);

			/// remove serviceGroup pair
			///@param	begin ServiceGroup����ʼ��
			///@param	end   ServiceGroup�Ľ�����
			///@return bool       �ɹ����� true, ʧ�ܷ��� false ��ע���쳣���
			bool remove(int begin, int end);

			/// �������Servicegroup
			void reset();

			///find sgId �Ƿ����
			///@return bool   ���� ����true,������ ����false
			bool find(int sgId);

			//�����ַ��� ����_sgMagane
			bool AnalyseBuf(char* buf, int len, char separator);

			//�������ϣ�ʹ�غϲ��֣�����һ��
			void SimplifySGs();
			
			//��ӡ��_sgMagane�е�����
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
