#ifndef ENGINE_GNS_HPP
#define ENGINE_GNS_HPP

#include "network.hpp"
#include <map>
#include <string>

// STEAM support IS UNTESTED
// and I'm using examples from the GNS github

namespace Network
{

    const uint16_t DEFAULT_SERVER_PORT = 27930;

    // for when this becomes a steam game
    struct SteamAppInfo
    {
        int app_id;
    };

    // steam connection
    class SteamDatagram
    {
    public:
        static void InitSteamDatagramConnectionSockets();
        static void ShutdownSteamDatagramConnectionSockets();
    };

    class GNSClient : public NetworkContext
    {  
    public:
        SteamNetworkingIPAddr lastAddr;
        ISteamNetworkingSockets *netface;
        HSteamNetConnection connection;
    	void PollIncomingMessages();
    	void PollLocalUserInput();
    	void OnSteamNetConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t *pInfo );
	    static GNSClient *callback_interface;
    	static void SteamNetConnectionStatusChangedCallback( SteamNetConnectionStatusChangedCallback_t *pInfo );
    	void PollConnectionStateChanges();

        void Init(const SteamNetworkingIPAddr &serverAddr);
        virtual void Tick();
        virtual void Stop(const char* reason);
    };

    class GNSListenServer : public NetworkContext
    {
    public:
        HSteamListenSocket socket;
        HSteamNetPollGroup group;
        struct Client
        {
		    std::string nickname;
        };
	    std::map< HSteamNetConnection, Client > clients;
        void SendStringToClient( HSteamNetConnection conn, const char *str );
        void SendStringToAllClients( const char *str, HSteamNetConnection except = k_HSteamNetConnection_Invalid );
	    void PollIncomingMessages();
	    void PollLocalUserInput();
	    void SetClientNick( HSteamNetConnection hConn, const char *nick );
        void OnSteamNetConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t *pInfo );	

        void Init(uint16_t port = DEFAULT_SERVER_PORT);
        virtual void Tick();
        virtual void Stop(const char* reason);

        static GNSListenServer *callback_interface;
        static void SteamNetConnectionStatusChangedCallback( SteamNetConnectionStatusChangedCallback_t *pInfo );
	    void PollConnectionStateChanges();
    };
};

#endif