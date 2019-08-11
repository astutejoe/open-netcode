#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Weapon.generated.h"

const float ADS_SPEED = 20.0f;

const float RECOIL_RECOVERY_SPEED = 4.0f;

UCLASS()
class FIRSTPERSONCLIENT_API AWeapon : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* root;

	UPROPERTY(Category = Weapon, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* mesh;

	UPROPERTY(Category = Weapon, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAudioComponent* sound_emitter;

	UPROPERTY(Category = Weapon, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UParticleSystemComponent* muzzle_flash;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float recoil;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float shot_cooldown;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float reload_cooldown;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int magazine_capacity;

	AWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool Fire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool Reload();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void HideMagazine();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ShowMagazine();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void WeaponFired();

	float shot_timer = 0.0f;
	bool reloading = false;
	float reload_timer = 0.0f;
	bool firing = false;

	float sway_speed = 0.5f;
	float sway_intensity = 0.3f;

	bool sway = false;
	FVector sway_offset = FVector::ZeroVector;

	//in order to track the ammo count we need this exposed to blueprints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int ammo_count;

	virtual void Tick(float DeltaTime) override;
};
