#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "PlayerPawn.generated.h"

UCLASS()
class FIRSTPERSONCLIENT_API APlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* capsule;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* mesh;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* camera;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* spine_reference;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* weapon_component;

	FVector velocity;
	bool jumping = false;
	bool grounded = true;
	bool is_falling = false;
	float health;

	AWeapon* weapon;

	APlayerPawn();
};
