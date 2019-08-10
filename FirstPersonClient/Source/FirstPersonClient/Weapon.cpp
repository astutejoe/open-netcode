#include "Weapon.h"

AWeapon::AWeapon()
{
	root = CreateDefaultSubobject<USceneComponent>(TEXT("CrouchCameraReference"));
	SetRootComponent(root);

	mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	mesh->SetupAttachment(root);
	

	sound_emitter = CreateDefaultSubobject<UAudioComponent>(TEXT("Shot Sound"));
	sound_emitter->SetAutoActivate(false);
	sound_emitter->SetupAttachment(mesh);

	muzzle_flash = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Muzzle Flash"));
	muzzle_flash->SetAutoActivate(false);
	muzzle_flash->bAllowRecycling = true;
	muzzle_flash->SetupAttachment(mesh);

	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	shot_timer = shot_cooldown * 3;
}

bool AWeapon::Fire()
{
	if (!reloading && shot_timer <= 0.0f && ammo_count > 0)
	{
		sound_emitter->Play();
		muzzle_flash->Activate(true);
		ammo_count--;
		shot_timer = shot_cooldown;

		firing = ammo_count > 0;

		WeaponFired();

		return true;
	}
	else if (!reloading && shot_timer <= 0.0f && ammo_count == 0)
	{
		/*int sound_index = FMath::RandRange(0, dry_fire_sounds.Num() - 1);
		sound_emitter->SetSound(dry_fire_sounds[sound_index]);
		sound_emitter->Play();

		shot_timer = shot_cooldown;*/

		firing = false;
	}

	return false;
}

bool AWeapon::Reload()
{
	if (!reloading)
	{
		reloading = true;
		reload_timer = reload_cooldown;

		return true;
	}

	return false;
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

