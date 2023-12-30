#include <iostream>
#include <strHelper.h>
#include "CommandCenter.h"
#include <boost/bind.hpp>

CommandCenter::CommandCenter(void)
{
	OptDescription opt;
	opt.addOptions("show help message")					
		("cmd","$","show command help message","command", true );
	regVerb( "help", opt , boost::bind( &CommandCenter::help, this , _1 ) );
}

CommandCenter::~CommandCenter(void)
{
}
int CommandCenter::help( OptResult& opts )
{
	std::string cmd = opts.as<std::string>("cmd");
	if( cmd == "$")
	{
		COMMANDMAP::const_iterator itCmd = mComMap.begin();
		for( ; itCmd != mComMap.end() ; itCmd ++ )
		{
			const CommandReceiverS& recv = itCmd->second;
			if( recv.size() > 0 )
			{
				printf("%s\t\t\t%s\n",itCmd->first.c_str() , recv[0].desc.getDescription().c_str());
			}
		}
	}
	else
	{
		COMMANDMAP::const_iterator itCmd = mComMap.find(cmd);
		if( itCmd != mComMap.end() )
		{
			const CommandReceiverS& recv = itCmd->second;
			if( recv.size() > 0 )
			{				
				printf( "USAGE: %s %s", itCmd->first.c_str() , recv[0].desc.format().c_str() );
			}
		}
		else
		{
			printf("no such command\n");
		}		
	}
	return 0;
}
int CommandCenter::run( const std::string& command )
{
	std::vector<std::string> cmds;
	
	ZQ::common::stringHelper::SplitString( command , cmds ," "," ","\"");
	if( cmds.size() <= 0 )
		return CMD_ERR_NOTKNOWN;
	std::string verb = cmds[0];
	cmds.erase(cmds.begin());

	COMMANDMAP::const_iterator it = mComMap.find( verb );
	if( it == mComMap.end() )
	{
		std::cout<<"no such command"<<std::endl;
		return CMD_ERR_NOROUTINE;
	}
	else
	{
		int ret = 0;
		const CommandReceiverS recvs = it->second;
		CommandReceiverS::const_iterator itReceiver = recvs.begin();
		for ( ; itReceiver != recvs.end() ; itReceiver++ )
		{
			OptResult result;
			if( ParseOptions( itReceiver->desc , result , cmds ) )
			{
				try
				{
					ret = (itReceiver->receiver)(result);
					if( ret != CMD_ERR_CONTINUE )
						break;
				}
				catch( const std::exception& ex)
				{
					std::cout<<ex.what()<<std::endl;
				}				
			}
			else
			{
				printf( "USAGE: %s %s", it->first.c_str() ,itReceiver->desc.format().c_str() );
			}
			break;//execute only one routine
		}
		return ret;
	}
}

void CommandCenter::regVerb( const std::string& verb , const OptDescription& desc , CommandReceiver receiver )
{
	COMMANDROUTINE routine;
	routine.desc = desc;
	routine.receiver = receiver;

	COMMANDMAP::iterator it = mComMap.find( verb );
	if( it == mComMap.end() )
	{
		
		CommandReceiverS recvs;
		recvs.insert(recvs.begin(),routine);
		mComMap.insert( COMMANDMAP::value_type(verb,recvs) );
	}
	else
	{
		it->second.insert(it->second.begin() , routine);
	}
}
