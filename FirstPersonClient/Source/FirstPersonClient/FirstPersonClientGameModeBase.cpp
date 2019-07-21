#include "FirstPersonClientGameModeBase.h"
#include "Async/Async.h"
#include "Containers/Queue.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

Object objects[MAX_OBJECTS];
ObjectInstance objects_instances[MAX_OBJECTS];

int objects_counter = 0;
int objects_instances_counter = 0;

int32 id = -1;

FDateTime last_packet = FDateTime::Now();
uint32 last_sequence = 0;

TQueue<Action> players_actions;

AFirstPersonClientGameModeBase::AFirstPersonClientGameModeBase()
{
	for (int i = 0; i < MAX_OBJECTS; i++) {
		objects[i].position[0] = 0.0;
		objects[i].position[1] = 0.0;
		objects[i].position[2] = 0.0;
		objects[i].rotation[0] = 0.0;
		objects[i].rotation[1] = 0.0;
		objects[i].id = NULL_ID;
	}

	PrimaryActorTick.bCanEverTick = true;
}

void AFirstPersonClientGameModeBase::Join()
{
	for (int tries = 0; tries < MAX_JOIN_TRIES && !StaticNetworking::Joined(); tries++)
	{
		//header
		uint8 join_buffer[sizeof(uint8)];
		join_buffer[PACKET_TYPE] = (uint8)PacketType::Join;

		StaticNetworking::SendData(join_buffer, sizeof(uint8));

		float wait_time = 0.0f;
		while (!StaticNetworking::Joined() && wait_time < MAX_WAIT_JOIN_RESPONSE)
		{
			while (!StaticNetworking::HasPendingData() && wait_time < MAX_WAIT_JOIN_RESPONSE)
			{
				FPlatformProcess::Sleep(CONNECTION_WAIT_TIME);
				wait_time += CONNECTION_WAIT_TIME;
			}

			if (StaticNetworking::HasPendingData())
			{
				uint8 id_buffer[RECEIVE_BUFFER_SIZE];
				memset(id_buffer, 0, RECEIVE_BUFFER_SIZE);

				int bytes_received = StaticNetworking::ReceiveData(id_buffer, RECEIVE_BUFFER_SIZE);

				if (bytes_received > 0 && id_buffer[PACKET_TYPE] == (uint8)PacketType::ID)
				{
					memcpy(&id, id_buffer + ID_HEADER_OFFSET, sizeof(int32));

					if (id >= 0)
					{
						StaticNetworking::Join();
					}
					else
					{
						/*//for future reference, it can be the following:
						switch (id)
						{
						case SERVER_FULL:
						break;
						}*/

						//something went wrong and we should stop waiting and try again
						break;
					}
				}
				else
				{
					FPlatformProcess::Sleep(CONNECTION_WAIT_TIME);
					wait_time += CONNECTION_WAIT_TIME;
				}
			}
		}
	}
}

void AFirstPersonClientGameModeBase::Synchronize()
{
	while (true)
	{
		while (!StaticNetworking::Joined())
		{
			FPlatformProcess::Sleep(CONNECTION_WAIT_TIME);
		}

		while (StaticNetworking::Joined())
		{
			//Receive world state from the server
			uint8 receive_buffer[RECEIVE_BUFFER_SIZE];
			memset(receive_buffer, 0, RECEIVE_BUFFER_SIZE);
			int bytes_received = StaticNetworking::ReceiveData(receive_buffer, RECEIVE_BUFFER_SIZE);

			if (bytes_received > 0) {
				switch (receive_buffer[PACKET_TYPE])
				{
				case (uint8)PacketType::RPC:
				{
					uint32 rpc_sequence;
					uint8 function_id;
					int32 object_id;
					uint32 data_length;
					uint8* data = nullptr;

					memcpy(&rpc_sequence, receive_buffer + sizeof(uint8), sizeof(uint32));
					memcpy(&function_id, receive_buffer + sizeof(uint8) + sizeof(uint32), sizeof(uint8));
					memcpy(&object_id, receive_buffer + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), sizeof(int32));
					memcpy(&data_length, receive_buffer + sizeof(uint8) + sizeof(uint32) + sizeof(uint8) + sizeof(int32), sizeof(uint32));

					if (data_length > 0)
					{
						data = (uint8*)malloc(data_length);
						memcpy(data, receive_buffer + sizeof(uint8) + sizeof(uint32) + sizeof(uint8) + sizeof(int32) + sizeof(uint32), data_length);
					}

					Action action;
					action.object_id = object_id;
					action.action_id = function_id;
					action.data = data;
					action.data_length = data_length;

					players_actions.Enqueue(action);
					break;
				}
				case (uint8)PacketType::Update:
					objects_counter = (bytes_received - UPDATE_HEADER_OFFSET) / sizeof(Object);
					memcpy(&objects, receive_buffer + UPDATE_HEADER_OFFSET, (bytes_received - UPDATE_HEADER_OFFSET));
					memcpy(&last_sequence, receive_buffer + 1, sizeof(uint32));
					break;
				}

				last_packet = FDateTime::Now();
			}
		}
	}
}

void AFirstPersonClientGameModeBase::BeginPlay()
{
#if ONLINE
	PRINT_DEBUG("Trying to connect to server...");

	/* This should be configurable or queried on some sort of server management */
	StaticNetworking::IP = "127.0.0.1";
	StaticNetworking::port = 3000;

	if (!StaticNetworking::Connected() && !StaticNetworking::Connect())
	{
		PRINT_DEBUG("Couldn't connect to server.");
	}
	else
	{
		PRINT_DEBUG("Connected to server.");
		PRINT_DEBUG("Joining server...");
		Async<void>(EAsyncExecution::Thread, AFirstPersonClientGameModeBase::Join);
	}

	Async<void>(EAsyncExecution::Thread, AFirstPersonClientGameModeBase::Synchronize);
#endif
}

void AFirstPersonClientGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (StaticNetworking::Joined())
	{
		if ((FDateTime::Now() - last_packet).GetTotalSeconds() >= SERVER_TIMEOUT)
		{
			StaticNetworking::Disconnect();
		}

		//Update online world simulation
		for (int i = 0; i < objects_counter; i++)
		{
			if (objects[i].id != id)
			{
				//check if object is already spawned
				bool spawned = false;
				int object_instance_id = -1;
				for (int j = 0; j < objects_instances_counter; j++)
				{
					if (objects[i].id == objects_instances[j].id)
					{
						spawned = true;
						object_instance_id = j;
						break;
					}
				}

				GEngine->AddOnScreenDebugMessage(4 + i, .1f, FColor::Blue, FString::Printf(TEXT("Object id %d position %f %f %f"), objects[i].id, objects[i].position[0], objects[i].position[1], objects[i].position[2]), false);

				//spawn new objects
				if (!spawned)
				{
					FVector position = FVector(objects[i].position[0], objects[i].position[1], objects[i].position[2]);
					FRotator rotator;

					FActorSpawnParameters ActorSpawnParameters;
					ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

					switch (objects[i].class_id)
					{
					case (uint8)ObjectClass::Player:
						rotator = FRotator(0.0f, objects[i].rotation[2], 0.0f);
						//objects_instances[objects_instances_counter].instance = GetWorld()->SpawnActor<AOnlinePawn>(OnlinePlayer, position, rotator, ActorSpawnParameters);
						break;
					default:
						break;
					}

					objects_instances[objects_instances_counter].id = objects[i].id;
					objects_instances_counter++;
				}
				//update already spawned objects
				else
				{
					FVector position = FVector(objects[i].position[0], objects[i].position[1], objects[i].position[2]);
					FRotator rotation;

					switch (objects[i].class_id)
					{
					case (uint8)ObjectClass::Player:
					{
						rotation = FRotator(0.0f, objects[i].rotation[2], 0.0f);

						AOnlinePawn* online_pawn_instance = Cast<AOnlinePawn>(objects_instances[object_instance_id].instance);

						if (online_pawn_instance != nullptr)
						{
							online_pawn_instance->Update(position, rotation, objects[i].velocity[0], objects[i].velocity[1], objects[i].grounded, objects[i].health);
							online_pawn_instance->spine_reference->SetRelativeRotation(FRotator(0.0f, 0.0f, objects[i].rotation[0]));

							if (objects[i].health <= 0.0f && !online_pawn_instance->ragdoll)
							{
								online_pawn_instance->EnableRagdoll();
							}

							if (objects[i].health > 0.0f && online_pawn_instance->ragdoll)
							{
								online_pawn_instance->DisableRagdoll();
							}
						}

						break;
					}
					default:
						break;
					}
				}
			}
		}

		for (int i = 0; i < MAX_ACTIONS_PER_TICK && !players_actions.IsEmpty(); i++)
		{
			Action action;
			players_actions.Dequeue(action);

			switch (action.action_id)
			{
			case (uint8)RPCAction::Fire:
			{
				for (int j = 0; j < objects_instances_counter; j++)
				{
					if (objects_instances[j].id == action.object_id)
					{
						Cast<AOnlinePawn>(objects_instances[j].instance)->Fire();
						break;
					}
				}
				break;
			}
			case (uint8)RPCAction::Reload:
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Player %u reloaded"), action.object_id), true);
				for (int j = 0; j < objects_instances_counter; j++)
				{
					if (objects_instances[j].id == action.object_id)
					{
						Cast<AOnlinePawn>(objects_instances[j].instance)->Reload();
						break;
					}
				}
				break;
			}
			case (uint8)RPCAction::Hit:
			{
				//in the future we may have blood effects, this calculates the opposite direction of bullet entry to emit a spur
				float impact_point[3];
				float impact_inverse_direction[3];
				memcpy(&impact_point[0], action.data, sizeof(float));
				memcpy(&impact_point[1], action.data + sizeof(float), sizeof(float));
				memcpy(&impact_point[2], action.data + sizeof(float) + sizeof(float), sizeof(float));

				memcpy(&impact_inverse_direction[0], action.data + sizeof(float) + sizeof(float) + sizeof(float), sizeof(float));
				memcpy(&impact_inverse_direction[1], action.data + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float), sizeof(float));
				memcpy(&impact_inverse_direction[2], action.data + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float), sizeof(float));

				FVector emit_location = FVector(impact_point[0], impact_point[1], impact_point[2]);
				FRotator emit_rotation = FRotator(impact_inverse_direction[0] - 90.0f, impact_inverse_direction[1], impact_inverse_direction[2]);
				FTransform emit_transform = FTransform(emit_rotation, emit_location);

				//UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), hit_blood_effect, emit_transform, true);
				break;
			}
			}

			if (action.data != nullptr)
				free(action.data);
		}

		for (int i = 0; i < objects_instances_counter; i++)
		{
			bool should_destroy = true;

			//if we have an instance of a object which isn't present in the world anymore, we should destroy that instance
			for (int j = 0; j < objects_counter; j++)
			{
				if (objects_instances[i].id == objects[j].id)
				{
					should_destroy = false;
				}
			}

			if (should_destroy)
			{
				if (objects_instances[i].instance != nullptr && objects_instances[i].instance->IsValidLowLevel())
					GetWorld()->DestroyActor(objects_instances[i].instance);

				for (int j = i; j < (objects_instances_counter - 1); j++)
				{
					objects_instances[j] = objects_instances[j + 1];
				}
				objects_instances_counter--;
				i--;
			}
		}
	}
}

ObjectHistory AFirstPersonClientGameModeBase::GetMyObject()
{
	ObjectHistory object_history;
	object_history.object.id = NULL_ID;

	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		if (objects[i].id == id) {
			object_history.object = objects[i];
			break;
		}
	}

	object_history.sequence = last_sequence;

	return object_history;
}