#include "FirstPersonServerGameModeBase.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Async/Async.h"
#include "Containers/Queue.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

uint8 receive_buffer[MAX_BUFFER];

Object objects[MAX_OBJECTS];
Object objects_history[MAX_OBJECTS_HISTORY][MAX_OBJECTS];
ObjectInstance objects_instances[MAX_OBJECTS];

Player players[MAX_PLAYERS];

TQueue<OnlinePlayerInputIded> inputs_queue;
TQueue<Player> new_players;
TQueue<Action> action_queue;

volatile int32 current_id = 0;

int objects_counter = 0;
volatile int players_counter = 0;
int history_counter = 0;

FCriticalSection id_mutex;
FCriticalSection player_mutex;

uint32 last_input_sequence[MAX_PLAYERS];

AFirstPersonServerGameModeBase::AFirstPersonServerGameModeBase()
{
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		objects[i].id = NULL_ID;

		objects[i].velocity[0] = 0.0f;
		objects[i].velocity[1] = 0.0f;
		objects[i].velocity[2] = 0.0f;

		objects[i].position[0] = 0.0f;
		objects[i].position[1] = 0.0f;
		objects[i].position[2] = 0.0f;

		objects[i].rotation[0] = 0.0f;
		objects[i].rotation[1] = 0.0f;
		objects[i].rotation[2] = 0.0f;

		objects[i].health = 100.0f;

		objects[i].class_id = (uint8)ObjectClass::Player;

		objects[i].grounded = false;

		objects[i].ads = false;

		objects[i].crouching = false;
	}

	PrimaryActorTick.bCanEverTick = true;
}

void AFirstPersonServerGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (StaticNetworking::BindPort(DEFAULT_GAME_PORT))
		PRINT_DEBUG("Succesfully bound to game port.");
	else
		PRINT_DEBUG("Failed to bind to game port.");

	Async<void>(EAsyncExecution::Thread, AFirstPersonServerGameModeBase::Listener);

	SpawnObject((uint8)ObjectClass::AICharacter, AICharacter, FVector::ZeroVector, FRotator::ZeroRotator, true, -1, 100.0f);

#if UE_BUILD_SHIPPING
	GetWorld()->GetGameViewport()->bDisableWorldRendering = true;
#endif
}

void AFirstPersonServerGameModeBase::Listener()
{
	FIPv4Endpoint source;

	while (true)
	{
		memset(receive_buffer, 0, MAX_BUFFER);

		if (StaticNetworking::ReceiveData(receive_buffer, MAX_BUFFER, &source))
		{
			int32 id;

			switch (receive_buffer[PacketStructure::Join::PACKET_TYPE])
			{
			case (uint8)PacketType::Join:
				id = AFirstPersonServerGameModeBase::GetIdFromAddr(source);

				//IF ID NOT FOUND, IT'S A NEW PLAYER
				PRINT_DEBUG("Player connection coming.");
				if (id < 0 && players_counter < MAX_PLAYERS)
				{
					PRINT_DEBUG("It's a new player.");

					id = AFirstPersonServerGameModeBase::AllocId();

					Player player;
					player.id = id;
					player.connection = source;
					player.last_packet = FDateTime::Now();
					player.latency = 0;

					player_mutex.Lock();
					players[players_counter] = player;
					players_counter++;
					player_mutex.Unlock();

					new_players.Enqueue(player);
				}
				else if (players_counter >= MAX_PLAYERS)
				{
					id = JoinErrorCode::ServerFull;
					PRINT_DEBUG("Server full, player couldn't be accepted");
				}

				if (id >= 0)
					players[AFirstPersonServerGameModeBase::GetPlayerIndexById(id)].last_packet = FDateTime::Now();

				uint8 idbuffer[PacketStructure::ID::SIZE];

				idbuffer[PacketStructure::ID::PACKET_TYPE] = (uint8)PacketType::ID;

				memcpy((idbuffer + PacketStructure::ID::ID_OFFSET), &id, sizeof(int32));

				StaticNetworking::SendData(idbuffer, PacketStructure::ID::SIZE, source);

				break;

			case (uint8)PacketType::Update:
				if ((id = AFirstPersonServerGameModeBase::GetIdFromAddr(source)) != -1)
				{
					OnlinePlayerInputIded player_input_ided;
					player_input_ided.id = id;

					memcpy(&player_input_ided.player_input, receive_buffer + PacketStructure::Update::HEADER_OFFSET, sizeof(OnlinePlayerInput));

					inputs_queue.Enqueue(player_input_ided);

					players[AFirstPersonServerGameModeBase::GetPlayerIndexById(id)].last_packet = FDateTime::Now();

					memcpy(&players[AFirstPersonServerGameModeBase::GetPlayerIndexById(id)].latency, receive_buffer + PacketStructure::Update::LATENCY_OFFSET, sizeof(uint32));
				}

				break;

			case (uint8)PacketType::RPC:
				if ((id = AFirstPersonServerGameModeBase::GetIdFromAddr(source)) != -1)
				{
					RPCDesc desc;
					memcpy(&desc, receive_buffer + PacketStructure::RPC::HEADER_OFFSET, sizeof(RPCDesc));

					uint8* data = nullptr;
					if (desc.data_length > 0)
					{
						data = new uint8[desc.data_length];
						memcpy(data, receive_buffer + PacketStructure::RPC::DATA_OFFSET, desc.data_length);
					}

					Action action;
					action.object_id = id;
					action.action_id = desc.function_id;
					action.data = data;

					action_queue.Enqueue(action);
				}
				break;

			default:
				break;
			}
		}
	}
}

int32 AFirstPersonServerGameModeBase::AllocId()
{
	id_mutex.Lock();

	int32 id = current_id;
	current_id++;

	id_mutex.Unlock();

	return id;
}

int32 AFirstPersonServerGameModeBase::GetIdFromAddr(FIPv4Endpoint addr)
{
	int32 id = -1;

	for (int i = 0; i < players_counter; i++)
	{
		if (players[i].connection.Address.Value == addr.Address.Value &&
			players[i].connection.Port == addr.Port)
		{
			id = players[i].id;
			break;
		}
	}

	return id;
}

int AFirstPersonServerGameModeBase::GetPlayerIndexById(int32 id)
{
	for (int i = 0; i < players_counter; i++)
		if (players[i].id == id)
			return i;

	return -1;
}

int AFirstPersonServerGameModeBase::GetObjectIndexById(int32 id)
{
	for (int i = 0; i < objects_counter; i++)
		if (objects[i].id == id)
			return i;

	return -1;
}

AActor* AFirstPersonServerGameModeBase::SpawnObject(uint8 class_id, UClass* object_class, FVector spawn_location, FRotator spawn_rotation, bool grounded = false, int32 id = -1, float health = 100.0f)
{
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	switch (class_id)
	{
	case (uint8)ObjectClass::Player:
	{
		APlayerPawn* player_instance = GetWorld()->SpawnActor<APlayerPawn>(object_class, spawn_location, spawn_rotation, ActorSpawnParameters);
		objects_instances[objects_counter].instance = player_instance;
		break;
	}
	case (uint8)ObjectClass::AICharacter:
	{
		objects_instances[objects_counter].instance = GetWorld()->SpawnActor<AAICharacter>(object_class, spawn_location, spawn_rotation, ActorSpawnParameters);
		break;
	}
	}

	objects_instances[objects_counter].class_id = class_id;

	Object object;
	object.velocity[0] = 0.0f;
	object.velocity[1] = 0.0f;
	object.velocity[2] = 0.0f;
	object.position[0] = spawn_location.X;
	object.position[1] = spawn_location.Y;
	object.position[2] = spawn_location.Z;
	object.rotation[0] = spawn_rotation.Roll;
	object.rotation[1] = spawn_rotation.Pitch;
	object.rotation[2] = spawn_rotation.Yaw;
	object.health = health;
	object.id = id == -1 ? AFirstPersonServerGameModeBase::AllocId() : id;
	object.class_id = class_id;
	object.grounded = grounded;

	objects[objects_counter] = object;

	if (class_id == (uint8)ObjectClass::Player)
	{
		APlayerPawn* player_instance = Cast<APlayerPawn>(objects_instances[objects_counter].instance);
		player_instance->SpawnDefaultController();
		player_instance->health = health;
		player_instance->object_id = id;
	}
	else if (class_id == (uint8)ObjectClass::AICharacter)
	{
		Cast<AAICharacter>(objects_instances[objects_counter].instance)->SpawnDefaultController();
		Cast<AAICharacter>(objects_instances[objects_counter].instance)->health = health;
		Cast<AAICharacter>(objects_instances[objects_counter].instance)->id = object.id;
	}

	objects_counter++;

	return objects_instances[objects_counter - 1].instance;
}

void AFirstPersonServerGameModeBase::DestroyObject(int object_index)
{
	if (object_index != -1)
	{
		GetWorld()->DestroyActor(objects_instances[object_index].instance);

		for (int j = object_index; j < (objects_counter - 1); j++)
		{
			objects_instances[j] = objects_instances[j + 1];
			objects[j] = objects[j + 1];
		}

		objects_counter--;
	}
}

void AFirstPersonServerGameModeBase::CleanupPlayers()
{
	for (int i = 0; i < players_counter; i++)
	{
		//verify timeout
		if ((FDateTime::Now() - players[i].last_packet).GetTotalSeconds() >= CLIENT_TIMEOUT)
		{
			PRINT_DEBUG(FString::Printf(TEXT("Kicking out player %d due to timeout"), players[i].id));

			int object_index = AFirstPersonServerGameModeBase::GetObjectIndexById(players[i].id);

			DestroyObject(object_index);

			player_mutex.Lock();
			for (int j = i; j < (players_counter - 1); j++)
			{
				players[j] = players[j + 1];
			}

			players_counter--;
			player_mutex.Unlock();

			i--;
		}
	}
}

void AFirstPersonServerGameModeBase::SpawnNewPlayers()
{
	if (!new_players.IsEmpty())
	{
		Player player;
		new_players.Dequeue(player);

		AActor* player_actor_instance = SpawnObject((uint8)ObjectClass::Player, PlayerPawn, player_spawn_location + FVector(0.0f, (100.0f * players_counter - 1), 0.0f), FRotator(0.0f, 0.0f, 0.0f), false, player.id);

		APlayerPawn* player_instance = Cast<APlayerPawn>(player_actor_instance);

		TSubclassOf<AWeapon>* player_weapon_class = weapons_map.Find(0);

		if (player_weapon_class != nullptr)
		{
			//as objects doesn't have begin plays nor tick we need to use a basic actor as the weapon
			player_instance->weapon = GetWorld()->SpawnActor<AWeapon>(*player_weapon_class, FVector::ZeroVector, FRotator::ZeroRotator);
			player_instance->weapon->ammo_count = player_instance->weapon->magazine_capacity;
		}
	}
}

void AFirstPersonServerGameModeBase::ResolvePlayerInput()
{
	for (int j = 0; j < MAX_ACTIONS_PER_TICK && !inputs_queue.IsEmpty(); j++)
	{
		OnlinePlayerInputIded online_player_input_ided;
		inputs_queue.Dequeue(online_player_input_ided);

		for (int i = 0; i < objects_counter; i++)
		{
			if (objects[i].id == online_player_input_ided.id)
			{
				int player_index = AFirstPersonServerGameModeBase::GetPlayerIndexById(online_player_input_ided.id);

				last_input_sequence[player_index] = online_player_input_ided.player_input.sequence;

				if (online_player_input_ided.player_input.crouching != objects[i].crouching)
				{
					APlayerPawn* player_instance = Cast<APlayerPawn>(objects_instances[i].instance);

					if (online_player_input_ided.player_input.crouching)
					{
						player_instance->spine_reference->SetRelativeLocation(player_instance->crouch_camera_location);
					}
					else
					{
						player_instance->spine_reference->SetRelativeLocation(player_instance->camera_location);
					}
				}

				float speed = JOG_SPEED;
				float acceleration = JOG_ACCELERATION;

				if (online_player_input_ided.player_input.crouching)
				{
					speed = CROUCH_SPEED;
					acceleration = CROUCH_ACCELERATION;
				}
				else if (online_player_input_ided.player_input.sprinting)
				{
					speed = SPRINT_SPEED;
					acceleration = SPRINT_ACCELERATION;
				}

				objects[i].crouching = online_player_input_ided.player_input.crouching;
				objects[i].ads = online_player_input_ided.player_input.ads;

				if (online_player_input_ided.player_input.move_forward != 0.0f)
				{
					objects[i].velocity[0] += acceleration * online_player_input_ided.player_input.move_forward;
				}
				else
				{
					if (objects[i].velocity[0] > 0.0f)
					{
						objects[i].velocity[0] -= acceleration * online_player_input_ided.player_input.delta_time;

						if (objects[i].velocity[0] < 0.0f)
							objects[i].velocity[0] = 0.0f;
					}
					else
					{
						objects[i].velocity[0] += acceleration * online_player_input_ided.player_input.delta_time;

						if (objects[i].velocity[0] > 0.0f)
							objects[i].velocity[0] = 0.0f;
					}
				}

				objects[i].velocity[0] = objects[i].velocity[0] > speed ? speed : objects[i].velocity[0];
				objects[i].velocity[0] = objects[i].velocity[0] < -speed ? -speed : objects[i].velocity[0];

				if (online_player_input_ided.player_input.move_right != 0.0f)
				{
					objects[i].velocity[1] += acceleration * online_player_input_ided.player_input.move_right;
				}
				else
				{
					if (objects[i].velocity[1] > 0.0f)
					{
						objects[i].velocity[1] -= acceleration * online_player_input_ided.player_input.delta_time;

						if (objects[i].velocity[1] < 0.0f)
							objects[i].velocity[1] = 0.0f;
					}
					else
					{
						objects[i].velocity[1] += acceleration * online_player_input_ided.player_input.delta_time;

						if (objects[i].velocity[1] > 0.0f)
							objects[i].velocity[1] = 0.0f;
					}
				}

				objects[i].velocity[1] = objects[i].velocity[1] > speed ? speed : objects[i].velocity[1];
				objects[i].velocity[1] = objects[i].velocity[1] < -speed ? -speed : objects[i].velocity[1];

				//Normalize the velocity so it's always length 1
				FVector2D velocity_normal = FVector2D(objects[i].velocity[0], objects[i].velocity[1]);

				if (velocity_normal.Size() > speed)
				{
					velocity_normal.Normalize();
					velocity_normal *= speed;
				}

				if (velocity_normal.X < 0.0f)
				{
					velocity_normal.X = velocity_normal.X / BACKWARD_COEFICIENT;
					velocity_normal.Y = velocity_normal.Y / BACKWARD_COEFICIENT;
				}

				APlayerPawn* player_instance = Cast<APlayerPawn>(objects_instances[i].instance);

				float movement_amount = velocity_normal.X * online_player_input_ided.player_input.delta_time;

				player_instance->MoveForward(movement_amount);

				UNavigationSystemV1* nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
				FNavLocation tmp;

				bool inside_navmesh = nav->ProjectPointToNavigation(player_instance->GetActorLocation(), tmp);

				//rollback to previous position
				if (!inside_navmesh)
				{
					player_instance->MoveForward(-movement_amount);
					objects[i].velocity[0] = 0.0f;
				}

				movement_amount = velocity_normal.Y * online_player_input_ided.player_input.delta_time;

				player_instance->MoveRight(movement_amount);

				inside_navmesh = nav->ProjectPointToNavigation(player_instance->GetActorLocation(), tmp);

				//rollback to previous position
				if (!inside_navmesh)
				{
					player_instance->MoveRight(-movement_amount);
					objects[i].velocity[1] = 0.0f;
				}

				if (online_player_input_ided.player_input.turn != 0.0f)
					player_instance->Turn(online_player_input_ided.player_input.turn);

				if (online_player_input_ided.player_input.turn_up != 0.0f)
					player_instance->TurnUp(online_player_input_ided.player_input.turn_up);

#pragma region JUMP

				const float GRAVITY = -980.f;

				objects[i].velocity[2] += GRAVITY * online_player_input_ided.player_input.delta_time * 2.0f;

				if (player_instance->grounded && online_player_input_ided.player_input.jumped)
				{
					objects[i].velocity[2] = 700.0f;
					player_instance->jumping = true;
				}

				const FCollisionShape& collisionShape = player_instance->capsule->GetCollisionShape();

				FNavLocation navmesh_location;
				nav->ProjectPointToNavigation(player_instance->GetActorLocation(), navmesh_location);

				FVector playerPositionOnNavMesh = navmesh_location.Location + FVector::UpVector * collisionShape.Capsule.HalfHeight;

				const float speed_z = objects[i].velocity[2] * online_player_input_ided.player_input.delta_time;
				const FVector movement = FVector::UpVector * speed_z;

				const bool is_falling = speed_z < 0.f;
				const FVector playerPosition = (player_instance->GetActorLocation() + movement);
				const float distanceFromGround = playerPosition.Z - playerPositionOnNavMesh.Z;

				const bool attemptSnapToGround = (is_falling || !player_instance->jumping);

				const float maxDownStep = 2.f;
				player_instance->grounded = attemptSnapToGround && distanceFromGround <= maxDownStep;

				if (player_instance->grounded)
				{
					player_instance->SetActorLocation(playerPositionOnNavMesh);

					objects[i].velocity[2] = 0.f;
					player_instance->jumping = false;
				}
				else
				{
					player_instance->MoveUp(speed_z);
				}
#pragma endregion

				player_instance->velocity = FVector(objects[i].velocity[0], objects[i].velocity[1], objects[i].velocity[2]);
				player_instance->ads = online_player_input_ided.player_input.ads;
				player_instance->crouching = online_player_input_ided.player_input.crouching;
				break;
			}
		}
	}
}

void AFirstPersonServerGameModeBase::ResolveActions()
{
	for (int i = 0; i < MAX_ACTIONS_PER_TICK && !action_queue.IsEmpty(); i++)
	{
		Action action;
		action_queue.Dequeue(action);

		int object_index = AFirstPersonServerGameModeBase::GetObjectIndexById(action.object_id);
		int player_index = AFirstPersonServerGameModeBase::GetPlayerIndexById(action.object_id);

		switch (action.action_id)
		{
		case (uint8)RPCAction::Fire:
		{
			PRINT_DEBUG(FString::Printf(TEXT("Player %u tried to fire"), objects[i].id));

			APlayerPawn* object_pawn = Cast<APlayerPawn>(objects_instances[object_index].instance);

			//if the gun can't fire, just break and ignore the action (could be due to reloading, cooldown or ammo)
			if (object_pawn == nullptr || object_pawn->weapon == nullptr || !object_pawn->weapon->Fire())
				break;

			PRINT_DEBUG(FString::Printf(TEXT("Player %u did fire"), objects[i].id));

			USceneComponent* exit_location = object_pawn->exit_location;

			FVector exit_direction;

			if (object_pawn->ads)
				exit_direction = exit_location->GetForwardVector();
			else
				exit_direction = (exit_location->GetComponentRotation() + FRotator(FMath::FRandRange(-HIPFIRE_SPREAD, HIPFIRE_SPREAD), FMath::FRandRange(-HIPFIRE_SPREAD, HIPFIRE_SPREAD), 0.0f)).Vector();

			FVector trace_start = exit_location->GetComponentLocation() + (exit_location->GetForwardVector() * WEAPON_LENGTH);
			FVector trace_end = trace_start + (exit_direction * MAX_SHOT_RANGE);

			FHitResult hit_out;

			FCollisionObjectQueryParams object_trace_params(
				ECC_TO_BITFIELD(ECC_WorldDynamic) |
				ECC_TO_BITFIELD(ECC_WorldStatic) |
				ECC_TO_BITFIELD(ECC_Pawn) |
				ECC_TO_BITFIELD(ECC_PhysicsBody) |
				ECC_TO_BITFIELD(ECC_Destructible)
			);

			FCollisionQueryParams trace_params(
				FName(TEXT("FireTrace")),
				true
			);

			/* roll back players to compensate for lag */

			int history_position = history_counter - (players[player_index].latency / 33.33f) - 1;

			if (history_position < 0)
				history_position += MAX_OBJECTS_HISTORY;

			FVector* tmp_positions = new FVector[objects_counter];

			for (int j = 0; j < objects_counter; j++)
			{
				FVector new_position = FVector(objects_history[history_position][j].position[0], objects_history[history_position][j].position[1], objects_history[history_position][j].position[2]);
				tmp_positions[j] = objects_instances[j].instance->GetActorLocation();
				objects_instances[j].instance->SetActorLocation(new_position, false);
			}

			/*******************************************/

			APlayerPawn* try_cast_player = nullptr;
			AAICharacter* try_cast_character = nullptr;

			bool hit_something = GetWorld()->LineTraceSingleByObjectType(hit_out, trace_start, trace_end, object_trace_params, trace_params);

			if (hit_something && hit_out.Actor != nullptr && hit_out.Actor->IsValidLowLevel())
			{
				try_cast_player = Cast<APlayerPawn>(hit_out.Actor.Get());
				try_cast_character = Cast<AAICharacter>(hit_out.Actor.Get());

				if (try_cast_player != nullptr)
				{
					if (try_cast_player->health > 0.0f)
					{
						try_cast_player->Hit(50.0f);//pending complex damage system
					}

					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, hit_out.BoneName.ToString(), true);
				}
				else if (try_cast_character != nullptr)
				{
					if (try_cast_character->health > 0.0f)
					{
						try_cast_character->health -= 50.0f;
					}

					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, hit_out.BoneName.ToString(), true);
				}
			}

			/* return players to original position */

			for (int j = 0; j < objects_counter; j++)
			{
				objects_instances[j].instance->SetActorLocation(tmp_positions[j], false);
			}

			delete tmp_positions;

			/***************************************/

			DrawDebugLine(GetWorld(), trace_start, trace_end, FColor::Red, false, 30.0f, ESceneDepthPriorityGroup::SDPG_World, 1);

			Cast<APlayerPawn>(objects_instances[object_index].instance)->TurnUp(-object_pawn->weapon->recoil);

			//replicate shot and hit
			for (int j = 0; j < players_counter; j++)
			{
				if (players[j].id != action.object_id)
					StaticNetworking::SendRPC((uint8)PacketType::RPC, action.action_id, action.object_id, 0, nullptr, players[j].connection);

				if (try_cast_player != nullptr)
				{
					uint32 data_length = sizeof(float) * 6;
					uint8 data[sizeof(float) * 6];

					memcpy(data, &hit_out.ImpactPoint.X, sizeof(float));
					memcpy(data + sizeof(float), &hit_out.ImpactPoint.Y, sizeof(float));
					memcpy(data + sizeof(float) + sizeof(float), &hit_out.ImpactPoint.Z, sizeof(float));

					FRotator hit_backward = (exit_location->GetForwardVector() * -1).Rotation(); //shot direction inverted

					memcpy(data + sizeof(float) + sizeof(float) + sizeof(float), &hit_backward.Pitch, sizeof(float));
					memcpy(data + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float), &hit_backward.Yaw, sizeof(float));
					memcpy(data + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float), &hit_backward.Roll, sizeof(float));

					StaticNetworking::SendRPC((uint8)PacketType::RPC, (uint8)RPCAction::Hit, try_cast_player->object_id, data_length, data, players[j].connection);
				}
			}

			break;
		}
		case (uint8)RPCAction::Reload:
		{
			PRINT_DEBUG(FString::Printf(TEXT("Player %u reloaded"), action.object_id));

			APlayerPawn* object_pawn = Cast<APlayerPawn>(objects_instances[object_index].instance);

			if (object_pawn != nullptr && object_pawn->weapon != nullptr)
			{
				object_pawn->weapon->Reload();

				for (int j = 0; j < players_counter; j++)
				{
					if (players[j].id != action.object_id)
						StaticNetworking::SendRPC((uint8)PacketType::RPC, (uint8)RPCAction::Reload, action.object_id, 0, nullptr, players[j].connection);
				}
			}

			break;
		}
		}

		if (action.data != nullptr)
			delete action.data;
	}
}

void AFirstPersonServerGameModeBase::UpdateWorldArray()
{
	for (int i = 0; i < objects_counter; i++)
	{
		FVector new_position = objects_instances[i].instance->GetActorLocation();
		FRotator new_rotation = objects_instances[i].instance->GetActorRotation();

		objects[i].position[0] = new_position.X;
		objects[i].position[1] = new_position.Y;
		objects[i].position[2] = new_position.Z;

		objects[i].rotation[1] = new_rotation.Pitch;
		objects[i].rotation[2] = new_rotation.Yaw;

		switch (objects_instances[i].class_id)
		{
		case (uint8)ObjectClass::Player:
			if (objects_instances[i].instance != nullptr)
			{
				APlayerPawn* player_instance = Cast<APlayerPawn>(objects_instances[i].instance);

				objects[i].rotation[0] = player_instance->spine_reference->GetComponentRotation().Pitch;
				objects[i].grounded = player_instance->grounded;
				objects[i].health = player_instance->health;
				objects[i].ads = player_instance->ads;
			}
			break;
		case (uint8)ObjectClass::AICharacter:
			if (objects_instances[i].instance != nullptr)
			{
				AAICharacter* character_instance = Cast<AAICharacter>(objects_instances[i].instance);

				objects[i].velocity[0] = character_instance->GetVelocity().X;
				objects[i].velocity[1] = character_instance->GetVelocity().Y;
				objects[i].velocity[2] = character_instance->GetVelocity().Z;
				objects[i].health = character_instance->health;
			}
			break;
		default:
			break;
		}

		GEngine->AddOnScreenDebugMessage(i+1, 2.f, FColor::Blue, FString::Printf(TEXT("Object id %d position %f %f %f velocity %f %f %f"), objects[i].id, objects[i].position[0], objects[i].position[1], objects[i].position[2], objects[i].velocity[0], objects[i].velocity[1], objects[i].velocity[2]), false);
	}
}

void AFirstPersonServerGameModeBase::DispatchWorldData()
{
	if (players_counter > 0)
	{
		const uint32 world_size = sizeof(Object) * objects_counter;
		uint8* send_buffer = new uint8[world_size + PacketStructure::Update::HEADER_OFFSET];
		memset(send_buffer, 0, world_size);
		send_buffer[PacketStructure::Update::PACKET_TYPE] = (uint8)PacketType::Update;

		for (int i = 0; i < players_counter; i++)
		{
			memcpy(send_buffer + 1, &(last_input_sequence[i]), sizeof(uint32));
			memcpy(send_buffer + PacketStructure::Update::HEADER_OFFSET, objects, world_size);
			StaticNetworking::SendData(send_buffer, world_size + PacketStructure::Update::HEADER_OFFSET, players[i].connection);
		}

		delete send_buffer;
	}
}

void AFirstPersonServerGameModeBase::Cleanup(float DeltaTime)
{
	//For now every object will be destroy upon death, we may make a switch over classes that won't inherit that behavior, while in the client-side the object will be replace by a dead one
	for (int i = 0; i < objects_counter; i++)
	{
		if (objects[i].class_id == (uint8)ObjectClass::Player && objects[i].health <= 0)
		{
			Cast<APlayerPawn>(objects_instances[i].instance)->respawn_counter -= DeltaTime;

			if (Cast<APlayerPawn>(objects_instances[i].instance)->respawn_counter <= 0)
			{
				objects_instances[i].instance->SetActorLocation(player_spawn_location, false);
				Cast<APlayerPawn>(objects_instances[i].instance)->Relive();
			}
		}
		else if (objects[i].health <= 0)
		{
			DestroyObject(i);
		}
	}
}

void AFirstPersonServerGameModeBase::Tick(float DeltaTime)
{
	GEngine->AddOnScreenDebugMessage(0, 2.f, FColor::Green, FString::Printf(TEXT("Players - Objects: %d - %d"),players_counter, objects_counter), false);

	CleanupPlayers();
	SpawnNewPlayers();

	ResolvePlayerInput();
	ResolveActions();
	UpdateWorldArray();

	memcpy(objects_history[history_counter], objects, sizeof(Object) * MAX_OBJECTS);
	history_counter = (history_counter + 1) % MAX_OBJECTS_HISTORY;

	DispatchWorldData();

	Cleanup(DeltaTime);
}