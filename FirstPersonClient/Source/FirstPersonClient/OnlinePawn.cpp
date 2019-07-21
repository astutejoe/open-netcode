#include "OnlinePawn.h"

AOnlinePawn::AOnlinePawn()
{
	capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	capsule->SetCapsuleHalfHeight(88.0f);
	capsule->SetCapsuleRadius(34.0f);
	SetRootComponent(capsule);

	mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	mesh->SetupAttachment(capsule);

	spine_reference = CreateDefaultSubobject<USceneComponent>(TEXT("Camera Placeholder"));
	spine_reference->SetupAttachment(mesh);

	camera_reference = CreateDefaultSubobject<USceneComponent>(TEXT("Camera Reference"));
	camera_reference->bEditableWhenInherited = true;
	camera_reference->SetupAttachment(spine_reference);

	weapon_component = CreateDefaultSubobject<UChildActorComponent>(TEXT("CAGAITA"));
	weapon_component->bEditableWhenInherited = true;
	weapon_component->SetupAttachment(camera_reference);

	PrimaryActorTick.bCanEverTick = true;

}

void AOnlinePawn::BeginPlay()
{
	Super::BeginPlay();

	weapon = Cast<AWeapon>(weapon_component->GetChildActor());
	if (weapon)
	{
		weapon->SetOwner(this);
	}

	target_location = GetActorLocation();
	target_rotation = GetActorRotation();
}

void AOnlinePawn::Fire()
{
	weapon->Fire();
	PlayerFired();
}

void AOnlinePawn::Reload()
{
	weapon->Reload();
	PlayerReloaded();
}

void AOnlinePawn::EnableRagdoll()
{
	mesh->SetSimulatePhysics(true);
	mesh->SetAllBodiesSimulatePhysics(true);
	mesh->WakeRigidBody();
	mesh->SetCollisionProfileName("Floor");

	ragdoll = true;
}

void AOnlinePawn::DisableRagdoll()
{
	mesh->SetSimulatePhysics(false);
	mesh->SetAllBodiesSimulatePhysics(false);
	mesh->PutRigidBodyToSleep();
	mesh->SetCollisionProfileName("NoCollision");

	mesh->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	mesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	ragdoll = false;
}

void AOnlinePawn::Update(FVector location, FRotator rotation, float forward_velocity, float side_velocity, bool _isInGround, float _health)
{
	target_location = location;
	target_rotation = rotation;
	velocity = FVector(forward_velocity, side_velocity, 0.0f);
	health = _health;
	grounded = _isInGround;
}

void AOnlinePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetActorLocation() != target_location)
	{
		SetActorLocation(FMath::VInterpTo(GetActorLocation(), target_location, DeltaTime, INTERP_SPEED));
	}

	if (GetActorRotation() != target_rotation)
	{
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), target_rotation, DeltaTime, INTERP_SPEED));
	}

	/*if (weapon != nullptr && weapon->reloading)
	{
		weapon_component->SetWorldLocation(mesh->GetSocketLocation("gun_socket"));
		weapon_component->SetWorldRotation(mesh->GetSocketQuaternion("gun_socket"));
	}*/
}

