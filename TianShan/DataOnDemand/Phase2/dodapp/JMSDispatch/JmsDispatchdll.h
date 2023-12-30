#ifdef MYJMSDISPATCH_EXPORTS
#define MYJMSDISPATCH_API __declspec(dllexport)
#else
#define MYJMSDISPATCH_API __declspec(dllimport)
#endif
extern "C" MYJMSDISPATCH_API
BOOL JmsPortInitialize(std::string JbossIpPort,
					   std::string ConfigQueueName,
					   std::string CachePath,
					   long  ConfigMsgTimeOut,
					   long  UsingJboss,
					   std::string DODEndPoint,
					   Ice::CommunicatorPtr ic,
					   ZQ::common::Log* pLog);

extern "C" MYJMSDISPATCH_API
BOOL JmsPortUnInitialize();