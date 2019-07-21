#pragma once

#include "Networking/Public/Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

#define LISTEN_IP TEXT("0.0.0.0")

class FIRSTPERSONSERVER_API StaticNetworking
{
private:
	static ISocketSubsystem* socket_subsystem;
	static FSocket* socket;

	static uint32 rpc_sequence;

public:
	static bool BindPort(int32 port);
	static bool SendData(uint8* buffer, int length, FIPv4Endpoint destination);
	static bool ReceiveData(uint8* buffer, int length, FIPv4Endpoint* source);
	static void SendRPC(uint8 packet_type, uint8 function_id, int32 object_id, const uint32 data_length, uint8* data, FIPv4Endpoint destination);
};
