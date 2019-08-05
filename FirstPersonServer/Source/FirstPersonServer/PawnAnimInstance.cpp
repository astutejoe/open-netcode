// Fill out your copyright notice in the Description page of Project Settings.


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

		SpineOffset = Owner->spine_reference->GetComponentRotation().Pitch * -1.0f;

		bIsGrounded = Owner->grounded;
		bIsJumping = Owner->jumping;
	}
}