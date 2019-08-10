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

	if (weapon != nullptr)
	{
		/*if (weapon->reloading)
		{
			weapon_component->SetWorldLocation(GetMesh()->GetSocketLocation("gun_socket"));
			weapon_component->SetWorldRotation(GetMesh()->GetSocketQuaternion("gun_socket"));

			if (camera->FieldOfView != camera_target_fov)
				camera->SetFieldOfView(FMath::FInterpTo(camera->FieldOfView, camera_target_fov, DeltaTime, ADS_SPEED / 2));

		}
		else */if (interpolate_weapon_location)
		{
			FVector weapon_relative_location = weapon_component->RelativeLocation;
			weapon_target_location.X = weapon_relative_location.X;

			weapon_component->SetRelativeLocation(FMath::VInterpTo(weapon_relative_location, weapon_target_location, DeltaTime, ADS_SPEED));

			camera->SetFieldOfView(FMath::FInterpTo(camera->FieldOfView, camera_target_fov, DeltaTime, ADS_SPEED));

			//GETTING ROTATION BACK DUE TO KNOWN BUG THAT CHANGING LOCATION AFFECTS ROTATION: https://issues.unrealengine.com/issue/UE-46037
			//Also to get back from reloads properly
			weapon_component->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

			if (weapon_relative_location.Equals(weapon_target_location))
			{
				interpolate_weapon_location = false;
			}
		}
		else if (!aiming_downsights && !(weapon_component->RelativeLocation.Equals(weapon_hipfire_location)))
		{
			FVector weapon_relative_location = weapon_component->RelativeLocation;
			FVector interp_location = weapon_hipfire_location;
			interp_location.X = weapon_relative_location.X;

			weapon_component->SetRelativeLocation(FMath::VInterpConstantTo(weapon_relative_location, interp_location, DeltaTime, ADS_SPEED * 2));

			camera->SetFieldOfView(FMath::FInterpTo(camera->FieldOfView, camera_target_fov, DeltaTime, ADS_SPEED / 2));

			//GETTING ROTATION BACK DUE TO KNOWN BUG THAT CHANGING LOCATION AFFECTS ROTATION: https://issues.unrealengine.com/issue/UE-46037
			//Also to get back from reloads properly
			weapon_component->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
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

	weapon_hipfire_location = weapon_component->RelativeLocation;

	fps_camera_location = camera->RelativeLocation;

	fps_crouch_camera_location = crouch_camera_reference->RelativeLocation;
}