#include "PlayerPawn.h"
#include "DrawDebugHelpers.h"

APlayerPawn::APlayerPawn()
{
	capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	capsule->InitCapsuleSize(34.0f, 88.0f);
	capsule->SetCanEverAffectNavigation(true);
	capsule->bDynamicObstacle = true;
	RootComponent = capsule;

	spine_reference = CreateDefaultSubobject<USceneComponent>(TEXT("SpineReference"));
	spine_reference->SetRelativeLocation(FVector(0.f, 0.f, 14.f));
	spine_reference->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	spine_reference->bEditableWhenInherited = true;
	spine_reference->SetupAttachment(capsule);

	mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	mesh->SetupAttachment(capsule);

	exit_location = CreateDefaultSubobject<USceneComponent>(TEXT("ExitLocation"), true);
	exit_location->SetRelativeLocation(FVector(29.0f, 6.0f, 36.0f));
	exit_location->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	exit_location->bEditableWhenInherited = true;
	exit_location->SetupAttachment(spine_reference);

	crouch_camera_reference = CreateDefaultSubobject<USceneComponent>(TEXT("CrouchCameraReference"));
	crouch_camera_reference->bEditableWhenInherited = true;
	crouch_camera_reference->SetupAttachment(capsule);
}

void APlayerPawn::MoveForward(float value)
{
	if (state != State::Dead)
	{
		if (value != 0)
		{
			AddActorLocalOffset(FVector::ForwardVector * value);
		}
	}
}

void APlayerPawn::MoveRight(float value)
{
	if (state != State::Dead)
	{
		if (value != 0)
		{
			AddActorLocalOffset(FVector::RightVector * value);
		}
	}
}

void APlayerPawn::MoveUp(float value)
{
	if (state != State::Dead)
	{
		AddActorLocalOffset(FVector::UpVector * value);
	}
}

void APlayerPawn::Turn(float value)
{
	if (state != State::Dead)
	{
		FRotator rotation = GetActorRotation();
		rotation.Yaw += value;

		SetActorRotation(rotation);
	}
}

void APlayerPawn::TurnUp(float value)
{
	if (state != State::Dead)
	{
		FRotator spine_reference_rotation = spine_reference->GetComponentRotation();

		spine_reference_rotation.Pitch = FMath::ClampAngle(spine_reference_rotation.Pitch - value, -MAX_ROTATION, MAX_ROTATION);
		spine_reference_rotation.Roll = 0.0f;
		spine_reference_rotation.Yaw = 0.0f;

		spine_reference->SetRelativeRotation(spine_reference_rotation);
	}
}

void APlayerPawn::Hit(float damage)
{
	health -= damage; //pending complex implementation

	DrawDebugCapsule(GetWorld(), GetActorLocation(), 88, 34, FQuat::Identity, FColor::Red, false, 30.0f, ESceneDepthPriorityGroup::SDPG_World, 1);

	if (health <= 0)
	{
		state = State::Dead;
	}
}

void APlayerPawn::Relive()
{
	health = 100.0f;
	state = State::Alive;
	respawn_counter = respawn_time;
}

void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	camera_location = spine_reference->RelativeLocation;
	crouch_camera_location = crouch_camera_reference->RelativeLocation;
}
