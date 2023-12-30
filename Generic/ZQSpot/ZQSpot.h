#ifndef _ZQSPOT_H_
#define _ZQSPOT_H_

#include "NativeThreadPool.h"
#include <Ice/Ice.h>
// #include "ZQSpotIce.h"

#ifdef ZQSPOT_EXPORTS
#define ZQSPOT_API __declspec(dllexport)
#else
// for test
// #define ZQSPOT_API 
#define ZQSPOT_API __declspec(dllimport)

#endif

#define MAX_LOAD		10000

#include <time.h>

//////////////////////////////////////////////////////////////////////////

namespace ZQ {
namespace Spot {

class ZQSPOT_API SpotEnv;
class ZQSPOT_API InterfaceLoadEvaluator;
class ZQSPOT_API InterfaceSelect;

class NodeSpot;
class SpotMgr;
class SpotStatusQuery;
class NodeWatchDog;
class SpotQueryImpl;
class LoadBindImpl;

// -----------------------------
// class InterfaceLoadEvaluator
// -----------------------------
/// the evaluator of interfaces

class InterfaceLoadEvaluator
{
public:
	/// the entry called by the adapter to calculate the load of an object
	virtual calcLoad(const Ice::ObjectPrx& obj)
	{
		// dummy Evaluator always return a fixed number between 0 and 10000
		return rand() % (MAX_LOAD / 2);
	}
};

// -----------------------------
// class InterfaceSelect
// -----------------------------
/// the selector of interfaces

class InterfaceSelect
{
public:

	typedef struct _InterfaceOption
	{
		std::string		endPoint;
		int				load;
	} InterfaceOption;

	typedef std::vector <InterfaceOption> InterfaceOptions;

	/// select an endpoint from the options by evaluating the given scores
	///@param options a list of potential interface providers
	///@return the selected interface
	virtual InterfaceOptions::iterator select(
		InterfaceOptions& options)
	{
		InterfaceOptions::iterator result = options.end();
		int minLoad = MAX_LOAD + 1;
		// the default dummy Select always choose the first available interface
		for (InterfaceOptions::iterator it = options.begin(); 
			it < options.end(); it++)
		{
			if (it->load <= MAX_LOAD && it->load < minLoad) {
				minLoad = it->load;
				result = it;
			}
		}
		
		return result;
	}
};

// -----------------------------
// class SpotEnv
// -----------------------------
/// spot environment

class SpotEnv {
public:
	/// the parameter of the spot environment for initialization
	struct InitParams {
		std::string		appName;			/// name of application
		std::string		iceServiceAddr;		/// ICE listening adrress
		unsigned short	iceServicePort;		/// ICE listening port
		std::string		nodeId;				/// identity of node
		std::string		multicastBindAddr;	/// binding address of multicast
		std::string		multicastGroupAddr;	/// group address
		unsigned short	multicastPort;		/// port of multicast
		bool			silent;				/// will spotEnv send heartbeat to other spotEnv
		std::string		logFilePath;		/// path of log file
	};

	SpotEnv(const InitParams& params);

	/// initialize the spot environment
	///@param[in]	argc		count of arguments
	///@param[in]	argv		arguments
	///@return		if the function succeeds, the return value is true.
	///				if the function fails, the return value is false.
	bool initialize(int argc , char* argv[]);

	/// shutdown the spot environment
	void shutdown();

	/// get the current ICE communicator
	///@return		the current ICE communicator
	const Ice::CommunicatorPtr getCommunicator();

	/// export a implementation of a interface
	///@param[in]	obj					pointer to the object whose interface is being exported
	///@param[in]	interfaceNamethe	name of interface
	///@param[in]	pEvaluator			evaluator of the interface
	///@return		if the function succeeds, the return value is true.
	///				if the function fails, the return value is false.
	bool exportInterface(::Ice::ObjectPtr obj, const std::string& interfaceName, 
		InterfaceLoadEvaluator* pEvaluator = NULL);

	/// open a implementation of a interface that is in the recored of 
	/// current spot environment
	///@param[in]	interfaceName		name of the interface.
	///@param[in]	sel					the selector of interfaces
	///@return		object found
	::Ice::ObjectPrx openInterface(const ::std::string& interfaceName, 
		InterfaceSelect* sel = NULL);

	/// get identity of current node
	///@return	id of current node
	const std::string& getNodeId();

	///@return	return a string to describe current machine
	const std::string& getCurrentMachineDesc();

	///@return	if the function succeeds, the return value is 
	///			the default evaluator of interface
	InterfaceLoadEvaluator* getDefaultEvaluator();

	/// call the function to set the default evaluator of interface
	///@param[in]	evaluator	default evaluator of interface
	void setDefaultEvaluator(InterfaceLoadEvaluator* evaluator);

	///@return	the status query of current spot environment
	SpotStatusQuery* getStatusQuery();
	
protected:
	friend class LoadBindImpl;
	friend class ZQ::Spot::NodeWatchDog;
	virtual ZQ::Spot::SpotMgr* getSpotMgr();

	Ice::ObjectPrx getSpotQueryFromEndPoint(std::string endpoint);
	const Ice::ObjectPrx& getLoadBind();

	SpotQueryImpl* getSpotQuery();
	
protected:
	Ice::CommunicatorPtr			_communicator;
	::Ice::ObjectAdapterPtr			_objAdapter;
	ZQ::common::NativeThreadPool	_threadPool;
	const InitParams				_initParams;
	ZQ::Spot::NodeSpot*				_nodeSpot;
	ZQ::Spot::SpotMgr*				_spotMgr;
	ZQ::Spot::SpotStatusQuery*		_spotData;
	SpotQueryImpl*					_spotQuery;
	std::string						_hostDesc;
	InterfaceLoadEvaluator*			_defLoadEvaluator;
	InterfaceSelect*				_defIfSel;
	bool							_hookIce;
	Ice::ObjectPrx					_loadBind;
	int								_processId;
};

} // namespace Spot
} // namespace ZQ

#endif // #ifndef _ZQSPOT_H_
