#include "PawnAnimInstance.h"

void UPawnAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* pawn = TryGetPawnOwner();

	if (pawn == nullptr) { return; }

	if (pawn->IsA(AOnlinePawn::StaticClass()))
	{
		Owner = Cast<AOnlinePawn>(pawn);
	}
	else if (pawn->IsA(APlayerPawn::StaticClass()))
	{
		LocalOwner = Cast<APlayerPawn>(pawn);
	}
}

void UPawnAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (Owner != nullptr)
	{
		Speed = Owner->velocity.Size();
		Direction = CalculateDirection(Owner->velocity, FRotator::ZeroRotator);
		bIsGrounded = Owner->grounded;
		bIsAiming = Owner->ads;
		Health = Owner->health;
		bIsCrouching = Owner->crouching;
		SpineOffset = Owner->spine_reference->GetComponentRotation().Pitch * -1.0f;

		if (Owner->weapon != nullptr)
			bIsReloading = Owner->weapon->reloading;
	}
	else if (LocalOwner != nullptr)
	{
		Speed = LocalOwner->velocity.Size();
		Direction = CalculateDirection(LocalOwner->velocity, FRotator::ZeroRotator);
		bIsGrounded = LocalOwner->grounded;
		bIsAiming = LocalOwner->aiming_downsights;
		Health = 100.0f;
		bIsCrouching = LocalOwner->crouching;
		SpineOffset = LocalOwner->spine_reference->GetComponentRotation().Pitch * -1.0f;

		if (LocalOwner->weapon != nullptr)
			bIsReloading = LocalOwner->weapon->reloading;
	}
}