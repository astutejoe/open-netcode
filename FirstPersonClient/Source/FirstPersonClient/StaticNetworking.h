#pragma once

#include "Networking/Public/Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

class FIRSTPERSONCLIENT_API StaticNetworking
{
private:
	static ISocketSubsystem* socket_subsystem;
	static FSocket* socket;
	static FIPv4Endpoint server_endpoint;
	static bool connected;
	static bool joined;
	static uint32 rpc_sequence;

public:
	static bool Connect();
	static bool SendData(uint8* buffer, int length);
	static int ReceiveData(uint8* buffer, int length);
	static bool HasPendingData();
	static bool Connected();
	static void Join();
	static bool Joined();
	static void Disconnect();
	static void SendRPC(uint8 packet_type, uint8 function_id, const uint32 data_length, uint8* data);

	static FString IP;
	static uint16 port;
};
