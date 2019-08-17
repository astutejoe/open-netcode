#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AICharacter.h"
#include "FirstPersonServerGameModeBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MainAIController.generated.h"

UENUM(BlueprintType)
enum class AIState : uint8
{
	Alive = 0 UMETA(DisplayName = "Alive"),
	Dead = 1 UMETA(DisplayName = "Dead")
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

	UFUNCTION(BlueprintCallable, Category = "AIController")
	void SetTarget(APawn* new_target);

	UFUNCTION(BlueprintCallable, Category = "AIController")
	void ShootTarget();

	const float AI_SPREAD = 5.0f;
	const float MAX_SHOT_RANGE = 4000.0f;
};
