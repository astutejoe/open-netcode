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

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

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

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* crouch_camera_reference;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PlayerReloaded();

	FVector velocity;
	bool jumping = false;
	bool crouching = false;
	bool sprinting = false;
	bool grounded = true;
	bool is_falling = false;
	float health;

	AWeapon* weapon;
	USkeletalMeshComponent* weapon_mesh;

	FVector camera_location;
	FVector crouch_camera_location;

	float camera_target_fov;
	FVector weapon_target_location;
	bool aiming_downsights = false;
	bool interpolate_weapon_location = false;

	APlayerPawn();
};
