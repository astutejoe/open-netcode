#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Weapon.generated.h"

UCLASS()
class FIRSTPERSONCLIENT_API AWeapon : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:	
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

	float shot_timer = 0.0f;
	bool reloading = false;
	float reload_timer = 0.0f;
	bool firing = false;

	//in order to track the ammo count we need this exposed to blueprints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int ammo_count;

	virtual void Tick(float DeltaTime) override;
};
