#ifndef __c2client_command_center_header_file_h__
#define __c2client_command_center_header_file_h__

#include <ZQ_common_conf.h>
#include <boost/function.hpp>
#include "PoormanOpt.h"

typedef boost::function< int ( OptResult& opts ) > CommandReceiver ;

class CommandCenter
{
public:
	CommandCenter(void);
	virtual ~CommandCenter(void);

public:

	int		run( const std::string& command );

	void	regVerb( const std::string& verb , const OptDescription& desc , CommandReceiver receiver );

	int		help( OptResult& opts );

private:
	
#ifdef ZQ_OS_LINUX
	#ifndef stricmp
		#define	stricmp strcasecmp
	#endif

	#ifndef strnicmp
		#define strnicmp strncasecmp
	#endif
#endif//ZQ_OS_LINUX

	class ICaseLess: std::binary_function<std::string, std::string, bool> 
	{
	public:
		result_type operator()( const first_argument_type& a, const second_argument_type& b) const
		{
			return (stricmp( a.c_str(), b.c_str()) < 0);
		}
	};
	struct COMMANDROUTINE 
	{
		CommandReceiver receiver;
		OptDescription	desc;
	};
	typedef std::vector<COMMANDROUTINE> CommandReceiverS;
	typedef std::map< std::string , CommandReceiverS  ,ICaseLess > COMMANDMAP;
	COMMANDMAP	mComMap;
};

#define CMD_ERR_QUIT			-10
#define CMD_ERR_NOROUTINE		-11
#define CMD_ERR_CONTINUE		-12
#define CMD_ERR_NOTKNOWN		-13

#endif//__c2client_command_center_header_file_h__
