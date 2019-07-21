#include "PawnAnimInstance.h"

void UPawnAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* pawn = TryGetPawnOwner();

	if (!pawn) { return; }

	if (pawn->IsA(APlayerPawn::StaticClass()))
	{
		Owner = Cast<APlayerPawn>(pawn);
	}
	else if (pawn->IsA(AOnlinePawn::StaticClass()))
	{
		RemoteOwner = Cast<AOnlinePawn>(pawn);
	}
}

void UPawnAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (Owner)
	{
		Speed = Owner->velocity.Size();
		Direction = CalculateDirection(Owner->velocity, FRotator::ZeroRotator);

		if (Owner->weapon != nullptr)
			bIsReloading = Owner->weapon->reloading;

		bIsGrounded = Owner->grounded;
		bIsFalling = Owner->is_falling;
		bIsJumping = Owner->jumping;
	}
	else if (RemoteOwner)
	{
		Speed = RemoteOwner->velocity.Size();
		Direction = CalculateDirection(RemoteOwner->velocity, FRotator::ZeroRotator);
		bIsGrounded = RemoteOwner->grounded;

		if (RemoteOwner->weapon != nullptr)
			bIsReloading = RemoteOwner->weapon->reloading;
	}
}