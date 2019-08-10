#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerPawn.h"
#include "OnlinePawn.h"
#include "PawnAnimInstance.generated.h"

UCLASS()
class FIRSTPERSONCLIENT_API UPawnAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFalling = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsGrounded = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsJumping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCrouching = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReloading = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAiming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Direction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpineOffset = 0.0f;

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	class AOnlinePawn* Owner = nullptr;
};
