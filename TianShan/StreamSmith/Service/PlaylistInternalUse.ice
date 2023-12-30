#ifndef __ZQ_TianShanIce_PlaylistInternalUse_ICE__
#define __ZQ_TianShanIce_PlaylistInternalUse_ICE__

#include "streamsmithadmin.ice"

module TianShanIce
{
module Streamer
{


["freeze:write"] 
class InternalPlaylistEx  extends  PlaylistEx
{
	///set path ticket string to playlist
	///@param pathticket ticket to be set
	void	setPathTicketProxy(TianShanIce::Transport::PathTicket* pathticket,InternalPlaylistEx* plEx);

	///renew ticket 
	///@param iTime renew time
	void	renewPathTicket( string ticketProxy , int iTime );

	///set playlist'data
	/// key ["PokeSessionID"] type [string] used for set NATPenetrating poke hole session id
	void	setPLaylistData(ValueMap plData)
			throws InvalidParameter,
			ServerError,
			InvalidStateOfArt;
	
	void	attachClientSessionId( string id );
	
	int		pushBackEx(int baseUserCtrlNum, PlaylistItemSetupInfoS newItemInfos)
				throws InvalidParameter,
					   ServerError,
					   InvalidStateOfArt;

};


};
};

#endif //__ZQ_TianShanIce_PlaylistInternalUse_ICE__
