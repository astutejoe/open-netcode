#include "StaticNetworking.h"
#include "FirstPersonClient.h"

FString StaticNetworking::IP;
uint16 StaticNetworking::port;

ISocketSubsystem* StaticNetworking::socket_subsystem = nullptr;
FSocket* StaticNetworking::socket = nullptr;

FIPv4Endpoint StaticNetworking::server_endpoint;

bool StaticNetworking::connected = false;
bool StaticNetworking::joined = false;

uint32 StaticNetworking::rpc_sequence = 0;

bool StaticNetworking::Connect()
{
	if (StaticNetworking::socket_subsystem == nullptr)
		StaticNetworking::socket_subsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	if (StaticNetworking::socket_subsystem != nullptr)
	{
		if (StaticNetworking::socket == nullptr)
			StaticNetworking::socket = StaticNetworking::socket_subsystem->CreateSocket(NAME_DGram, TEXT("Client Socket"), true);

		if (StaticNetworking::socket != nullptr)
		{
			FIPv4Address addr;
			FIPv4Address::Parse(StaticNetworking::IP, addr);

			StaticNetworking::server_endpoint = FIPv4Endpoint(addr, StaticNetworking::port);

			StaticNetworking::connected = StaticNetworking::socket->Connect(*StaticNetworking::server_endpoint.ToInternetAddr());

			if (StaticNetworking::connected)
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

bool StaticNetworking::Connected()
{
	return StaticNetworking::connected;
}

bool StaticNetworking::SendData(uint8* buffer, int length)
{
	if (StaticNetworking::connected)
	{
		int32 sent_bytes;
		bool success = StaticNetworking::socket->SendTo(buffer, length, sent_bytes, *StaticNetworking::server_endpoint.ToInternetAddr());

		if (!success || sent_bytes != length)
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

int StaticNetworking::ReceiveData(uint8* buffer, int length)
{
	int32 received_bytes;
	if (StaticNetworking::connected)
	{
		TSharedRef<FInternetAddr> source_shared_ref = socket_subsystem->CreateInternetAddr();

		bool success = StaticNetworking::socket->RecvFrom(buffer, length, received_bytes, *source_shared_ref, ESocketReceiveFlags::None);

		if (!success || received_bytes <= 0)
		{
			received_bytes = -1;
		}
	}
	else
	{
		received_bytes = -1;
	}

	return received_bytes;
}

bool StaticNetworking::HasPendingData()
{
	uint32 pending_data_size;
	return StaticNetworking::socket->HasPendingData(pending_data_size);
}

void StaticNetworking::Join()
{
	StaticNetworking::joined = true;
}

bool StaticNetworking::Joined()
{
	return StaticNetworking::joined;
}

void StaticNetworking::Disconnect()
{
	StaticNetworking::connected = false;
	StaticNetworking::joined = false;
	StaticNetworking::socket_subsystem->DestroySocket(StaticNetworking::socket);
	StaticNetworking::socket_subsystem = nullptr;
	StaticNetworking::socket = nullptr;
}

void StaticNetworking::SendRPC(uint8 packet_type, uint8 function_id, const uint32 data_length, uint8* data)
{
	int buffer_size = sizeof(uint8) + sizeof(uint32) + sizeof(uint8) + sizeof(uint32) + data_length; //packet_type + rpc_sequence + function_id + data_length + data
	uint8* rpc_buffer = (uint8*)malloc(buffer_size);

	//packet_type
	memcpy(rpc_buffer, &packet_type, sizeof(uint8));

	//rpc_sequence
	memcpy(rpc_buffer + sizeof(uint8), &rpc_sequence, sizeof(uint32));

	//function_id
	memcpy(rpc_buffer + sizeof(uint8) + sizeof(uint32), &function_id, sizeof(uint8));

	//data_length
	memcpy(rpc_buffer + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), &data_length, sizeof(uint32));

	//data
	if (data_length > 0)
		memcpy(rpc_buffer + sizeof(uint8) + sizeof(uint32) + sizeof(uint8) + sizeof(uint32), data, data_length);

	StaticNetworking::SendData(rpc_buffer, buffer_size);

	rpc_sequence = rpc_sequence + 1 % TNumericLimits<uint32>::Max();
}