// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerPawn.h"
#include "AICharacter.h"
#include "PawnAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPERSONSERVER_API UPawnAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsGrounded = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsJumping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsReloading = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsCrouching = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Direction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float SpineOffset = 0.0f;

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	APlayerPawn* Owner = nullptr;
	AAICharacter* AIOwner = nullptr;
	
};
