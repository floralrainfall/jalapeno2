#ifndef ENGINE_NETWORK_HPP
#define ENGINE_NETWORK_HPP

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

namespace Network
{
    enum ConnectionType
    {
        CT_CLIENT,
        CT_SERVER,
    };

    enum ConnectionStatus
    {
        CS_UNKNOWN,
        CS_CONNECTING,
        CS_CONNECTED,
        CS_CONNECTIONFAIL,
        CS_DISCONNECTED,
    };

    class NetworkContext
    {
    public:
        ConnectionStatus cstatus;
        ConnectionType ctype;
        ISteamNetworkingSockets *netface;
        bool active; // currently connected
        virtual void Tick() = 0;
        virtual void Stop(const char* reason) = 0;
    };
};

#endif