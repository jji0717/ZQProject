#ifndef __LSC_PROTOCOL_MESSAGE_PARSER_HEADER_FILE_H__
#define __LSC_PROTOCOL_MESSAGE_PARSER_HEADER_FILE_H__

#include "lsc_protocol.h"
#include <exception>
#include "lsc_common.h"
namespace lsc
{

class errorMessageException : public std::exception
{
public:
	errorMessageException(char* p) throw()
	{
        if(p&&p[0])
			msg = p;
	}
	virtual const char* what( ) const throw()
	{
		return msg.c_str();
	}
	virtual ~errorMessageException() throw()
	{
	}

private:
	std::string msg;
};

#ifdef INOUT
#undef INOUT
#endif
#define INOUT

class lscMessage;
///parse lsc message 
///@return the lscMessage instance pointer if success,NULL if failed or not complete
///NOTE if it's a bad request,errorMessageException will be throw out
///@param buf the byte buffer need to be decoded
///@param size byte buffer size
///@param lastMsg last incomplete message ,You should save lastMsg if it's not NULL
lscMessage*		ParseMessage(INOUT void*& buf,INOUT int& size,INOUT lscMessage*& lastMsg );

class lscMessage 
{
	friend lsc::lscMessage*	lsc::ParseMessage(void*& buf,int& size,lscMessage*& lastMsg );	
public:

	lscMessage();
	virtual ~lscMessage();

public:	
	void						hton();
	
	void						setStreamHandle( uint32_t strmHandle );

	void						setTransactionId( uint8_t transId );

	void						setStatusCode( uint8_t statusCode );

	void						setVersion( uint8_t version );

	void						setOpCode( uint8_t opCode );

	virtual uint32_t             toMetaData(StringMap& metadata);
	virtual bool                 readMetaData( const StringMap& metadata);
	
	const lsc::LSCMESSAGE&		GetLscMessageContent( ) const;
	
	const lsc::OperationCode	GetLscMessageOpCode( ) const;
	
	virtual bool				Parse();

	void						CopyFrom(const lscMessage& msg);
	
	unsigned int				getMsgSize();

	void						setMessageContent( const lsc::LSCMESSAGE& msg )
	{
		_message = msg;
	}
	void						setOpCode( lsc::OperationCode opCode )
	{
		_opCode = opCode;
	}
protected:
	uint32_t                    toLscHeader(StringMap& metadata);
	bool                        readLscHeader(const StringMap& metadata);
protected:
	
	lsc::LSCMESSAGE			_message;

	lsc::OperationCode		_opCode;

	///identify current message is complete or not ?
	bool					_bCompleteHeader;
	bool					_bCompleteBody;
	
	//How many bytes in current message
	int						_ownByteSize;
};

class lscResponse : public lscMessage
{
public:
	void		setCurrentNpt(const uint32_t curNpt)
	{
		_message.response.data.currentNpt = curNpt;
	}
	void		setScale(const  int16_t numerator ,const uint16_t denominator)
	{
		_message.response.data.numerator = numerator;
		_message.response.data.denominator = denominator;
	}
	void		setMode(const uint8_t mode)
	{
		_message.response.data.mode = mode;
	}

	uint32_t	getCurrentNpt( ) const
	{
		return _message.response.data.currentNpt;
	}
	void		getScale( int16_t& numerator , uint16_t& denominator ) const
	{
		numerator = _message.response.data.numerator;
		denominator = _message.response.data.denominator;
	}
	uint8_t		getMode( ) const
	{
		return _message.response.data.mode;
	}
	virtual uint32_t               toMetaData(StringMap& metadata);
	virtual bool                 readMetaData(const StringMap& metadata);
};

class lscPlay : public lscMessage
{
public:
	void		setStartNpt(const uint32_t startNpt)
	{
		_message.play.data.startNpt = startNpt;
	}
	void		setStopNpt(const uint32_t stopNpt)
	{
		_message.play.data.stopNpt = stopNpt;
	}
	void		setScale(const int16_t numerator , const uint16_t denominator)
	{
		_message.play.data.numerator = numerator;
		_message.play.data.denominator =denominator;
	}	
	uint32_t	getStartNpt( ) const
	{
		return _message.play.data.startNpt;
	}
	uint32_t	getStopNpt( ) const
	{
		return _message.play.data.stopNpt;
	}
	void		getScale(int16_t& numerator , uint16_t& denominator)
	{
		numerator = _message.play.data.numerator;
		denominator = _message.play.data.denominator;
	}
	virtual uint32_t               toMetaData(StringMap& metadata);
	virtual bool                 readMetaData(const StringMap& metadata);
};

typedef lscPlay lscJump;

class lscPause : public lscMessage
{
public:
	void		setStopNpt(const uint32_t stopNpt)
	{
		_message.pause.stopNpt =stopNpt;
	}
	uint32_t	getStopNpt( ) const
	{
		return _message.pause.stopNpt;
	}
	virtual uint32_t               toMetaData(StringMap& metadata);
	virtual bool                 readMetaData(const StringMap& metadata);
};

class lscResume : public lscMessage
{
public:

	void		setStartNpt(const uint32_t startNpt)
	{
		_message.resume.startNpt = startNpt;
	}

	void		setScale(const int16_t numerator , const uint16_t denominator)
	{
		_message.resume.numerator = numerator;
		_message.resume.denominator = denominator;
	}

	uint32_t	getStartNpt( )  const
	{
		return _message.resume.startNpt;
	}

	void		getScale(int16_t& numerator , uint16_t& denominator)
	{
		numerator = _message.resume.numerator;
		denominator = _message.resume.denominator;
	}
	virtual uint32_t               toMetaData(StringMap& metadata);
	virtual bool                 readMetaData(const StringMap& metadata);
};

class lscStatus: public lscMessage
{
public:
};

class lscReset : public lscMessage
{
public:
};

}
#endif//__LSC_PROTOCOL_MESSAGE_PARSER_HEADER_FILE_H__
