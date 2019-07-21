#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class FIRSTPERSONSERVER_API AWeapon : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AWeapon();
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float shot_cooldown;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int magazine_capacity;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float reload_cooldown;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float recoil;

	virtual void Tick(float DeltaTime) override;

	bool Fire();
	void Reload();

	float shot_timer = 0.0f;
	int ammo_count;
	bool reloading = false;
	float reload_timer = 0.0f;
};
