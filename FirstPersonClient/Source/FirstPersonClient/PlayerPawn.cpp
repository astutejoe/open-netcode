#include "PlayerPawn.h"

APlayerPawn::APlayerPawn()
{
	capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	capsule->SetCapsuleHalfHeight(88.0f);
	capsule->SetCapsuleRadius(34.0f);
	SetRootComponent(capsule);

	spine_reference = CreateDefaultSubobject<USceneComponent>(TEXT("SpineReference"));
	spine_reference->bEditableWhenInherited = true;
	spine_reference->SetupAttachment(capsule);

	mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	mesh->SetCastShadow(false);
	mesh->SetupAttachment(spine_reference);

	camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	camera->bEditableWhenInherited = true;
	camera->SetupAttachment(spine_reference);

	weapon_component = CreateDefaultSubobject<UChildActorComponent>(TEXT("CAGAITA")); //really sorry about this, calling it Weapon was actually giving compile errors.
	weapon_component->bEditableWhenInherited = true;
	weapon_component->SetupAttachment(camera);

	crouch_camera_reference = CreateDefaultSubobject<USceneComponent>(TEXT("CrouchCameraReference"));
	crouch_camera_reference->bEditableWhenInherited = true;
	crouch_camera_reference->SetupAttachment(capsule);
}

void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (camera->FieldOfView != camera_target_fov)
		camera->SetFieldOfView(FMath::FInterpTo(camera->FieldOfView, camera_target_fov, DeltaTime, ADS_SPEED / 2));

	if (weapon != nullptr)
	{
		if (weapon->reloading)
		{
			weapon_component->SetWorldLocation(FMath::VInterpConstantTo(weapon_component->GetComponentLocation(), mesh->GetSocketLocation("gun_socket"), DeltaTime, ADS_SPEED * 8));
			weapon_component->SetWorldRotation(FMath::RInterpConstantTo(weapon_component->GetComponentRotation(), mesh->GetSocketQuaternion("gun_socket").Rotator(), DeltaTime, ADS_SPEED * 8));
		}
		else if (aiming_downsights && !weapon_component->RelativeLocation.Equals(weapon_target_location))
		{
			weapon_component->SetRelativeLocation(FMath::VInterpTo(weapon_component->RelativeLocation, weapon_target_location, DeltaTime, ADS_SPEED));
		}
		else if (!aiming_downsights)
		{
			weapon_component->SetWorldLocation(FMath::VInterpConstantTo(weapon_component->GetComponentLocation(), mesh->GetSocketLocation("fps_gun_socket"), DeltaTime, ADS_SPEED*8));
			weapon_component->SetWorldRotation(FMath::RInterpConstantTo(weapon_component->GetComponentRotation(), mesh->GetSocketQuaternion("fps_gun_socket").Rotator(), DeltaTime, ADS_SPEED*8));
		}
	}
}

void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	weapon = Cast<AWeapon>(weapon_component->GetChildActor());
	if (weapon != nullptr)
	{
		weapon->SetOwner(this);
		weapon_mesh = weapon->mesh;
	}

	camera_location = spine_reference->RelativeLocation;
	crouch_camera_location = crouch_camera_reference->RelativeLocation;

	camera_target_fov = camera->FieldOfView;
}