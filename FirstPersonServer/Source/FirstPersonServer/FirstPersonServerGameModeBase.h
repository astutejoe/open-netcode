#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StaticNetworking.h"
#include "PlayerPawn.h"
#include "AICharacter.h"
#include "FirstPersonServerGameModeBase.generated.h"

const unsigned int NULL_ID = -1;

UENUM(BlueprintType)
enum class ObjectClass : uint8
{
	Player = 0 UMETA(DisplayName = "Player"),
	AICharacter = 1 UMETA(DisplayName = "AICharacter")
};

enum class RPCAction : uint8
{
	Fire = 0 UMETA(DisplayName = "Fire"),
	Reload = 1 UMETA(DisplayName = "Reload"),
	Hit = 3 UMETA(DisplayName = "Hit")
};

enum class PacketType : uint8
{
	Join = 0 UMETA(DisplayName = "Join"),
	ID = 1 UMETA(DisplayName = "Identifier"),
	RPC = 2 UMETA(DisplayName = "Remote Procedure Call"),
	Update = 3 UMETA(DisplayName = "World Update")
};

const unsigned short MAX_OBJECTS = 100;
const unsigned short MAX_PLAYERS = 15;

const unsigned short MAX_ACTIONS_PER_TICK = 10;
const unsigned short MAX_INPUTS_PER_TICK = 10;

const unsigned short MAX_BUFFER = 2048; //arbitrary

const unsigned short MAX_OBJECTS_HISTORY = 1000;

const float SPRINT_SPEED = 600.0f; //cm/s
const float SPRINT_ACCELERATION = 1200.0f; //cm/s
const float JOG_SPEED = 400.0f; //cm/s
const float JOG_ACCELERATION = 800.0f; //cm/s
const float CROUCH_SPEED = 200.0f; //cm/s
const float CROUCH_ACCELERATION = 400.0f; //cm/s

const float DEACELERATION_COEFICIENT = 4.0f;
const float BACKWARD_COEFICIENT = 2.0f;

const uint16 DEFAULT_GAME_PORT = 3000;

const float CLIENT_TIMEOUT = 60.0f; //seconds

const float WEAPON_LENGTH = 70.0f;
const float MAX_SHOT_RANGE = 4000.0f;

const float HIPFIRE_SPREAD = 3.0f; //ANGLES OF RANDOMNESS

#pragma pack(push,1)
struct Object
{
	float position[3];
	float rotation[3];
	float velocity[3];
	float health;
	int32 id;
	uint8 class_id;
	bool grounded;
	bool ads;
	bool crouching;
};
#pragma pack(pop)

#pragma pack(push,1)
struct OnlinePlayerInput
{
	float move_forward = 0.0f;
	float move_right = 0.0f;
	float turn = 0.0f;
	float turn_up = 0.0f;
	bool crouching = false;
	bool jumped = false;
	bool sprinting = false;
	bool ads = false;
	float delta_time = 0.0f;
	uint32 sequence = 0;
};
#pragma pack(pop)

struct OnlinePlayerInputIded
{
	OnlinePlayerInput player_input;
	int32 id;
};

#pragma pack(push,1)
struct RPCDesc
{
	uint32 rpc_sequence;
	uint8 function_id;
	uint32 data_length;
};
#pragma pack(pop)

struct ObjectInstance
{
	int class_id;
	AActor* instance;
};

struct Player
{
	int32 id;
	FIPv4Endpoint connection;
	OnlinePlayerInput player_input;
	FDateTime last_packet;
	uint32 latency;
};

struct Action
{
	uint8 action_id;
	int32 object_id;
	uint8* data;
};

#define PRINT_DEBUG(MESSAGE) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, MESSAGE, false)

namespace PacketStructure
{
	namespace Join
	{
		const uint32 PACKET_TYPE = 0;
		const uint32 ID = 1;
	}

	namespace ID
	{
		const uint32 PACKET_TYPE = 0;
		const uint32 ID_OFFSET = sizeof(uint8);
		const uint32 SIZE = sizeof(uint8) + sizeof(int32);
	}

	namespace Update
	{
		const uint32 PACKET_TYPE = 0;
		const unsigned short LATENCY_OFFSET = sizeof(uint8);
		const unsigned short HEADER_OFFSET = LATENCY_OFFSET + sizeof(uint32); //PACKET TYPE + LAST INPUT SEQUENCE (SERVER TO CLIENT) OR LATENCY (CLIENT TO SERVER)
	}

	namespace RPC
	{
		const unsigned short HEADER_OFFSET = sizeof(uint8);
		const unsigned short DATA_OFFSET = HEADER_OFFSET + sizeof(RPCDesc);
	}
}

namespace JoinErrorCode
{
	const int32 ServerFull = -1;
}

UCLASS()
class FIRSTPERSONSERVER_API AFirstPersonServerGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AFirstPersonServerGameModeBase();

	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector player_spawn_location;

	static void Listener();
	static int32 GetIdFromAddr(FIPv4Endpoint addr);
	static int32 AllocId();
	static int GetPlayerIndexById(int32 id);
	static int GetObjectIndexById(int32 id);
	void ReplicateShot(int object_index);
	void ReplicateHit(int object_id, FHitResult hit_out, FRotator hit_backward);

	AActor* SpawnObject(uint8 class_id, UClass* object_class, FVector spawn_location, FRotator spawn_rotation, bool grounded, int32 id, float health);
	void DestroyObject(int object_index);

	UPROPERTY(EditDefaultsOnly, Category = "GameDefaults")
	UClass* PlayerPawn;

	UPROPERTY(EditDefaultsOnly, Category = "GameDefaults")
	UClass* AICharacter;

	UPROPERTY(EditDefaultsOnly, Category = "DayDGameModeBase")
	TMap<uint8, TSubclassOf<AWeapon>> weapons_map;

private:
	void CleanupPlayers();
	void SpawnNewPlayers();
	void ResolvePlayerInput();
	void ResolveActions();
	void UpdateWorldArray();
	void DispatchWorldData();
	void Cleanup(float DeltaTime);
};
