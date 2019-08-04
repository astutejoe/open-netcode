#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "OnlinePawn.generated.h"

const float INTERP_SPEED = 30.0f;

UCLASS()
class FIRSTPERSONCLIENT_API AOnlinePawn : public APawn
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* capsule;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* weapon_component;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* mesh;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* camera_reference;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* spine_reference;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PlayerFired();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PlayerReloaded();

	AWeapon* weapon;

	AOnlinePawn();

	void Fire();
	void Reload();
	void EnableRagdoll();
	void DisableRagdoll();

	bool ads = false;
	bool interpolate_weapon_location = false;
	FVector weapon_hipfire_location;
	FVector weapon_ads_location = FVector(5.999996, 25.999794, 41.0);
	FVector weapon_target_location;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "OnlinePawn")
	virtual void Update(FVector location, FRotator rotation, float forward_velocity, float side_velocity, bool _isInGround, float _health, bool _ads);

	FVector target_location;
	FRotator target_rotation;

	bool grounded = true;
	float health;
	FVector velocity;
	bool ragdoll;
};
