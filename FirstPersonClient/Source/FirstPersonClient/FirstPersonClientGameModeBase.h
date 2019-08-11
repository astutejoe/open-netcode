#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StaticNetworking.h"
#include "OnlinePawn.h"
#include "FirstPersonClientGameModeBase.generated.h"

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

typedef struct object_instance {
	int32 id;
	AActor* instance;
} ObjectInstance;

typedef struct action {
	uint8 action_id;
	int32 object_id;
	uint8* data;
	uint32 data_length;
} Action;

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

typedef struct object_history {
	Object object;
	OnlinePlayerInput player_input;
	uint32 sequence;
	double timestamp;
} ObjectHistory;

#define UPDATE_HEADER_OFFSET 5
#define RPC_HEADER_OFFSET 1
#define ID_HEADER_OFFSET 1

#define RECEIVE_BUFFER_SIZE ((sizeof(Object) * MAX_OBJECTS) + UPDATE_HEADER_OFFSET)

//PACKET STRUCTURE
#define PACKET_TYPE 0
#define ACTION_TYPE 1

enum class PacketType : uint8
{
	Join = 0 UMETA(DisplayName = "Join"),
	ID = 1 UMETA(DisplayName = "Identifier"),
	RPC = 2 UMETA(DisplayName = "Remote Procedure Call"),
	Update = 3 UMETA(DisplayName = "World Update")
};

const unsigned int NULL_ID = -1;
const float CONNECTION_WAIT_TIME = 0.1f;
const float SERVER_TIMEOUT = 60.0f; //seconds
const float MAX_ROTATION = 60.0f;
const float MAX_WAIT_JOIN_RESPONSE = 10.0f;
const unsigned short MAX_OBJECTS = 100;
const unsigned short MAX_PLAYERS = 15;
const unsigned short MAX_ACTIONS_PER_TICK = 10;
const unsigned short MAX_JOIN_TRIES = 10;

#define ONLINE 1
#define PRINT_DEBUG(MESSAGE) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, MESSAGE, false)

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

extern int32 id;

UCLASS()
class FIRSTPERSONCLIENT_API AFirstPersonClientGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:
	AFirstPersonClientGameModeBase();

	void Tick(float DeltaTime) override;
	static void Join();
	static void Synchronize();
	static ObjectHistory GetMyObject();

	UPROPERTY(EditDefaultsOnly, Category = "Defaults")
	UClass* OnlinePlayer;
};
