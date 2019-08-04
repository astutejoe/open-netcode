#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "PlayerPawn.generated.h"

UENUM(BlueprintType)
enum class State : uint8
{
	Alive = 0				UMETA(DisplayName = "Alive"),
	Dead = 9				UMETA(DisplayName = "Dead")
};

#define MAX_ROTATION 60.0f

UCLASS()
class FIRSTPERSONSERVER_API APlayerPawn : public APawn
{
	GENERATED_BODY()

public:

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* capsule;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* mesh;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* spine_reference;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* exit_location;

	APlayerPawn();

	const int respawn_time = 10.0f;

	float health = 100.0f;

	State state = State::Alive;

	FVector velocity = FVector::ZeroVector;
	bool ads;
	bool grounded = false;
	bool jumping = false;

	AWeapon* weapon;

	float respawn_counter = respawn_time;

	void MoveForward(float value);
	void MoveRight(float value);
	void MoveUp(float value);
	void Turn(float value);
	void TurnUp(float value);

	void Hit(float damage);

	void Relive();
};
