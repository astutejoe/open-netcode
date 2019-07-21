#include "PlayerPawn.h"

APlayerPawn::APlayerPawn()
{
	capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	capsule->SetCapsuleHalfHeight(88.0f);
	capsule->SetCapsuleRadius(34.0f);
	SetRootComponent(capsule);

	mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	mesh->SetupAttachment(capsule);

	spine_reference = CreateDefaultSubobject<USceneComponent>(TEXT("SpineReference"));
	spine_reference->bEditableWhenInherited = true;
	spine_reference->SetupAttachment(mesh);

	camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	camera->bEditableWhenInherited = true;
	camera->SetupAttachment(spine_reference);

	weapon_component = CreateDefaultSubobject<UChildActorComponent>(TEXT("CAGAITA")); //really sorry about this, calling it Weapon was actually giving compile errors.
	weapon_component->bEditableWhenInherited = true;
	weapon_component->SetupAttachment(camera);
}

void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	weapon = Cast<AWeapon>(weapon_component->GetChildActor());
	if (weapon != nullptr)
	{
		weapon->SetOwner(this);
	}
}