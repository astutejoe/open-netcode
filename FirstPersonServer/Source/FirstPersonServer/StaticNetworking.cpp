#include "StaticNetworking.h"

ISocketSubsystem* StaticNetworking::socket_subsystem = nullptr;
FSocket* StaticNetworking::socket = nullptr;
uint32 StaticNetworking::rpc_sequence = 0;

bool StaticNetworking::BindPort(int32 port)
{
	FString IP = LISTEN_IP;

	StaticNetworking::socket_subsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	if (StaticNetworking::socket_subsystem != nullptr)
	{
		StaticNetworking::socket = StaticNetworking::socket_subsystem->CreateSocket(NAME_DGram, TEXT("Server Socket"), true);

		if (StaticNetworking::socket != nullptr)
		{
			FIPv4Address addr;
			FIPv4Address::Parse(IP, addr);

			FIPv4Endpoint endpoint = FIPv4Endpoint(addr, port);

			bool bound = StaticNetworking::socket->Bind(*endpoint.ToInternetAddr());

			if (bound)
			{
				StaticNetworking::socket->SetNonBlocking(false);
				StaticNetworking::socket->SetRecvErr();
			}
			else
			{
				return false;
			}

		}
		else
		{
			return false;
		}

	}
	else
	{
		return false;
	}

	return true;
}

bool StaticNetworking::SendData(uint8* buffer, int length, FIPv4Endpoint destination)
{
	int32 sent_bytes;
	bool success = StaticNetworking::socket->SendTo(buffer, length, sent_bytes, *destination.ToInternetAddr());

	if (!success || sent_bytes != length)
	{
		return false;
	}

	return true;
}

bool StaticNetworking::ReceiveData(uint8* buffer, int length, FIPv4Endpoint* source)
{
	int32 received_bytes;

	TSharedRef<FInternetAddr> source_shared_ref = socket_subsystem->CreateInternetAddr();

	bool success = StaticNetworking::socket->RecvFrom(buffer, length, received_bytes, *source_shared_ref, ESocketReceiveFlags::None);

	*source = FIPv4Endpoint(source_shared_ref);

	if (!success || received_bytes <= 0)
	{
		return false;
	}

	return true;
}

void StaticNetworking::SendRPC(uint8 packet_type, uint8 function_id, int32 object_id, const uint32 data_length, uint8* data, FIPv4Endpoint destination)
{
	int buffer_size = sizeof(uint8) + sizeof(uint32) + sizeof(uint8) + sizeof(int32) + sizeof(uint32) + data_length; //packet_type + rpc_sequence + function_id + object_id + data_length + data
	uint8* rpc_buffer = (uint8*)malloc(buffer_size);

	//packet_type
	memcpy(rpc_buffer, &packet_type, sizeof(uint8));

	//rpc_sequence
	memcpy(rpc_buffer + sizeof(uint8), &rpc_sequence, sizeof(uint32));

	//function_id
	memcpy(rpc_buffer + sizeof(uint8) + sizeof(uint32), &function_id, sizeof(uint8));

	//object_id
	memcpy(rpc_buffer + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), &object_id, sizeof(int32));

	//data_length
	memcpy(rpc_buffer + sizeof(uint8) + sizeof(uint32) + sizeof(uint8) + sizeof(int32), &data_length, sizeof(uint32));

	//data
	if (data_length > 0)
		memcpy(rpc_buffer + sizeof(uint8) + sizeof(uint32) + sizeof(uint8) + sizeof(int32) + sizeof(uint32), data, data_length);

	StaticNetworking::SendData(rpc_buffer, buffer_size, destination);

	rpc_sequence = rpc_sequence + 1 % TNumericLimits<uint32>::Max();
}