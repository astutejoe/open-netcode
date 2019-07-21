#include "Weapon.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	shot_timer = shot_cooldown * 3;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (shot_timer > 0.0f)
		shot_timer -= DeltaTime;

	if (reload_timer > 0.0f)
	{
		reload_timer -= DeltaTime;

		if (reload_timer <= 0.0f)
		{
			reloading = false;
			ammo_count = magazine_capacity;
		}
	}
}

bool AWeapon::Fire()
{
	if (!reloading && shot_timer <= 0.0f && ammo_count > 0.0f)
	{
		shot_timer = shot_cooldown;
		ammo_count--;

		return true;
	}

	return false;
}

void AWeapon::Reload()
{
	reloading = true;
	reload_timer = reload_cooldown;
}