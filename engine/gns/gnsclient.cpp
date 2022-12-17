#include "gns.hpp"
#include "../engine.hpp"

using namespace Network;

void GNSClient::Init(const SteamNetworkingIPAddr &serverAddr)
{
    netface = SteamNetworkingSockets();
    char saddr[ SteamNetworkingIPAddr::k_cchMaxString ];
    serverAddr.ToString( saddr, sizeof(saddr), true );
    engine_app->Logf("GNSClient: connecting to server at %s", saddr);		
    SteamNetworkingConfigValue_t opt;
	opt.SetPtr( k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)SteamNetConnectionStatusChangedCallback );
    ctype = CT_CLIENT;
    cstatus = CS_CONNECTING;
    connection = netface->ConnectByIPAddress(serverAddr, 1, &opt);
    lastAddr = serverAddr;
    if(connection == k_HSteamNetConnection_Invalid)
        engine_app->Logf("GNSClient: failed to create connection");
    active = true;
}

void GNSClient::Tick()
{
    if(active)
    {
        PollIncomingMessages();
        PollConnectionStateChanges();
        PollLocalUserInput();
    }
}

void GNSClient::PollIncomingMessages()
{
    ISteamNetworkingMessage *incoming = nullptr;
    int numMsgs = netface->ReceiveMessagesOnConnection( connection, &incoming, 1 );
    if(numMsgs == 0)
        return;
    else if(numMsgs < 0)
        engine_app->Logf("GNSClient: error checking messages");
    else
    {
        incoming->Release();
    }
}

void GNSClient::PollConnectionStateChanges()
{
    callback_interface = this;
    netface->RunCallbacks();
}

void GNSClient::PollLocalUserInput()
{

}

void GNSClient::Stop(const char* reason)
{
    netface->CloseConnection(connection, 0, reason, true);
    active = false;
    cstatus = CS_DISCONNECTED;
}

void GNSClient::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo)
{
    switch ( pInfo->m_info.m_eState )
    {
        case k_ESteamNetworkingConnectionState_None:
            // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
            break;
        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
            engine_app->Logf("GNSClient: Connection closed");

            // Print an appropriate message
            if ( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting )
            {
                // Note: we could distinguish between a timeout, a rejected connection,
                // or some other transport problem.
                engine_app->Logf( "GNSClient: We sought the remote host, yet our efforts were met with defeat.  (%s)", pInfo->m_info.m_szEndDebug );
            }
            else if ( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally )
            {
                engine_app->Logf( "GNSClient: Alas, troubles beset us; we have lost contact with the host.  (%s)", pInfo->m_info.m_szEndDebug );
            }
            else
            {
                // NOTE: We could check the reason code for a normal disconnection
                engine_app->Logf( "GNSClient: The host hath bidden us farewell.  (%s)", pInfo->m_info.m_szEndDebug );
            }

			netface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
			connection = k_HSteamNetConnection_Invalid;
            active = false;
            cstatus = CS_CONNECTIONFAIL;
            break;
		case k_ESteamNetworkingConnectionState_Connecting:
            cstatus = CS_CONNECTING;
            break;
        case k_ESteamNetworkingConnectionState_Connected:
            cstatus = CS_CONNECTED;
            engine_app->Logf("GNSClient: Connected");
            break;
		default:
            break;
    }
}

GNSClient *GNSClient::callback_interface = nullptr;
void GNSClient::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *pInfo)
{
    callback_interface->OnSteamNetConnectionStatusChanged(pInfo);
}