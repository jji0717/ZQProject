#include "VSISConnFactory.h"
#include "VSISCacheIO.h"

namespace CacheManagement {

uint64 VSISConnDialog::_recvdataNo = 0;

VSISConnDialog::VSISConnDialog(uint32 cfgRecvBuffSize)
{
	_cfgRecvBuffSize = cfgRecvBuffSize;
}

VSISConnDialog::~VSISConnDialog()
{
}


void VSISConnDialog::onCommunicatorSetup(ZQ::DataPostHouse::IDataCommunicatorPtr communicator)
{
	// remember this socket object as need to use in onRead()
	_clientSocket = ZQ::DataPostHouse::AClientSocketTcpPtr::dynamicCast(communicator);

	glog(ZQ::common::Log::L_INFO, CLOGFMT(VSISConnDialog, "VSISConnDialog::onCommunicatorSetup() with socket=%d "), 
		_clientSocket->getCommunicatorDescriptor());

}

void VSISConnDialog::onCommunicatorDestroyed(ZQ::DataPostHouse::IDataCommunicatorPtr communicator)
{
	// get the tcp client socket
	ZQ::DataPostHouse::AClientSocketTcpPtr sclient = ZQ::DataPostHouse::AClientSocketTcpPtr::dynamicCast(communicator);
	if(NULL == sclient || _clientSocket.get() != sclient.get())		// do a check, it should not be null, but anyway
	{	
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(VSISConnDialog, "VSISConnDialog::onCommunicatorDestroyed() return an invalid AClientSocketTcp object"));
		return;
	}

	// retrieve the VSISEnity 
	VSISEntityPtr pVSISEntity = VSISEntityPtr::dynamicCast(sclient->getUserData());
	if(NULL == pVSISEntity)	// do a check, it should not be null, but anyway
	{	
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(VSISConnDialog, "VSISConnDialog::onCommunicatorDestroyed() AClientSocketTcp object's UserData is wrong"));
		return;
	}
	
	glog(ZQ::common::Log::L_ERROR, CLOGFMT(VSISConnDialog, "VSISConnDialog::onCommunicatorDestroyed() connection to cluster %s node ip %s lost, will reconnect it soon"), 
		pVSISEntity->_clusterID.c_str(), pVSISEntity->_ip.c_str());

	// set node connection state to be disconnected
	_clientSocket = NULL;
	pVSISEntity->onConnectLost();
}

//
// this callback routine need to always return true;
//
bool VSISConnDialog::onRead(const int8* buffer, size_t bufSize)
{
	// for test
//	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISConnDialog, "VSISConnDialog::onRead() received data # %llu"), ++_recvdataNo);

	if(NULL == _clientSocket)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(VSISConnDialog, "VSISConnDialog::onRead() client socket object is NULL"));
		
		return true; // return true, otherwise the base class will not continue to read data
	}
	if(bufSize > _cfgRecvBuffSize)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(VSISConnDialog, "VSISConnDialog::onRead() received buff size is %d bytes, larger than max buffer size %d bytes, please check conifguraiton"), 
			bufSize, MAX_SOCKET_BUFF_SIZE);
		
		return true; // return true, otherwise the base class will not continue to read data
	}

	// retrieve the MediaCluster Node object
	VSISEntityPtr pVSISEntity = VSISEntityPtr::dynamicCast(_clientSocket->getUserData());
	if(NULL == pVSISEntity)	// do a check, it should not be null, but anyway
	{	
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(VSISConnDialog, "VSISConnDialog::onRead() AClientSocketTcp object's UserData is wrong"));
		return true;
	}
	
	// delivery the data 
	pVSISEntity->onDataReceived(buffer, bufSize);

	return true;
}

void VSISConnDialog::onWritten(size_t bufSize)
{
	// do nothing here
}

void VSISConnDialog::onError()
{
}

VSISConnFactory::VSISConnFactory()
{
	_connectionCount = 0;
}

VSISConnFactory::~VSISConnFactory()
{
}


ZQ::DataPostHouse::IDataDialogPtr VSISConnFactory::onCreateDataDialog(ZQ::DataPostHouse::IDataCommunicatorPtr communicator)
{	
	// check if one connection was asking for create multiple dialogs
	ZQ::common::MutexGuard guard(_dlgLock);
	if( _dlgs.find(communicator) != _dlgs.end() ) {
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(VSISConnFactory,"Logic error, trying to bind one communicator on two dialogs"));
		return NULL;
	}

	// create the dialog object
	ZQ::DataPostHouse::IDataDialogPtr dlg = new VSISConnDialog(_readBuffSize);		
	if(NULL == dlg) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(VSISConnFactory, "Create CMEConnDialog objct failed"));
		return NULL;
	}
	// save the communication and dialog object
	_dlgs.insert(std::make_pair<>(communicator,dlg));
	
	// log total dialog count
	_connectionCount++;
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISConnFactory, "VSISConnFactory::onCreateDataDialog() created dialog count %d"), 
		_connectionCount);

	return dlg;
}

void VSISConnFactory::onReleaseDataDialog(ZQ::DataPostHouse::IDataDialogPtr dialog, ZQ::DataPostHouse::IDataCommunicatorPtr communicator)
{	
	ZQ::common::MutexGuard guard(_dlgLock);
	DialogMap::iterator it = _dlgs.find(communicator);
	
	if ( it == _dlgs.end() )
	{
		return;
	}
	// erase the dialog 	
	_dlgs.erase(it);

	// log total dialog count
	_connectionCount--;
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISConnFactory, "VSISConnFactory::onReleaseDataDialog() current dialog count %d"), 
		_connectionCount);
}

void VSISConnFactory::onClose(IDataDialogFactory::CommunicatorS& comms)
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(VSISConnFactory, "VSISConnFactory::onClose(): current Dialog count is %d when this dialog creator close"), 
		comms.size());

	// erase this dig from my list
	IDataDialogFactory::CommunicatorS::iterator it = comms.begin();
	for(; it != comms.end(); it++ )
	{
		(*it)->close();
	}
	ZQ::common::MutexGuard guard(_dlgLock);
	_dlgs.clear();
}


}