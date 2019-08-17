#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AICharacter.h"
#include "FirstPersonServerGameModeBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MainAIController.generated.h"

UENUM(BlueprintType)
enum class EAIMode : uint8
{
	Idle = 0 UMETA(DisplayName = "Idle"),
	Engaged = 1 UMETA(DisplayName = "Engaged"),
	Dead = 2 UMETA(DisplayName = "Dead")
};

UCLASS()
class FIRSTPERSONSERVER_API AMainAIController : public AAIController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;

	AAICharacter* pawn = nullptr;
	APawn* target = nullptr;
	const FName target_key = "Target";
	const FName mode_key = "Mode";

	bool IsValidPawn(APawn* pawn_to_test);
	void SetTarget(APawn* new_target);

	UFUNCTION(BlueprintCallable, Category = "AIController")
	void AddTarget(APawn* new_target);

	UFUNCTION(BlueprintCallable, Category = "AIController")
	void RemoveTarget(APawn* removing_target);

	UFUNCTION(BlueprintCallable, Category = "AIController")
	void ShootTarget();

	UFUNCTION(BlueprintCallable, Category = "AIController")
	void Reload();

	UFUNCTION(BlueprintCallable, Category = "AIController")
	void SetMode(EAIMode new_mode);

	const float AI_SPREAD = 5.0f;
	const float MAX_SHOT_RANGE = 4000.0f;

	TArray<APawn*> targets;
	EAIMode mode = EAIMode::Idle;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PawnDied();
};
