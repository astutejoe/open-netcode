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
		SpineOffset = Owner->spine_reference->GetComponentRotation().Roll;

		if (Owner->weapon != nullptr)
			bIsReloading = Owner->weapon->reloading;
	}
}